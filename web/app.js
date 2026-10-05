(() => {
  "use strict";

  const $ = (selector) => document.querySelector(selector);
  const $$ = (selector) => [...document.querySelectorAll(selector)];

  const ui = {
    runtimeBadge: $("#runtimeBadge"),
    newGameButton: $("#newGameButton"),
    sceneTitle: $("#sceneTitle"),
    narrative: $("#narrative"),
    phaseBadge: $("#phaseBadge"),
    playerName: $("#playerName"),
    healthText: $("#healthText"),
    healthBar: $("#healthBar"),
    goldStat: $("#goldStat"),
    potionStat: $("#potionStat"),
    victoryStat: $("#victoryStat"),
    swordChip: $("#swordChip"),
    keyChip: $("#keyChip"),
    gateChip: $("#gateChip"),
    enemyPanel: $("#enemyPanel"),
    enemyName: $("#enemyName"),
    enemyHealthText: $("#enemyHealthText"),
    enemyHealthBar: $("#enemyHealthBar"),
    enemyGlyph: $("#enemyGlyph"),
    actionHeading: $("#actionHeading"),
    explorationActions: $("#explorationActions"),
    battleActions: $("#battleActions"),
    eventLog: $("#eventLog"),
    startScreen: $("#startScreen"),
    startForm: $("#startForm"),
    startButton: $("#startButton"),
    nameInput: $("#nameInput"),
    progressSteps: $$("#progressSteps .progress-step")
  };

  let wasm = null;
  let game = null;
  let lastMessage = "";
  let currentState = null;

  function state() {
    return JSON.parse(game.stateJson());
  }

  function phaseLabel(phase) {
    return {
      exploring: "EXPLORING",
      battle: "IN COMBAT",
      defeat: "DEFEAT",
      victory: "VICTORY"
    }[phase] || phase.toUpperCase();
  }

  function sceneTitleFor(snapshot) {
    if (snapshot.phase === "battle") {
      return snapshot.enemyName === "Dragon"
        ? "The Dragon has arrived"
        : `${snapshot.enemyName} encounter`;
    }
    if (snapshot.phase === "victory") {
      return "The road is yours";
    }
    if (snapshot.phase === "defeat") {
      return "Your run has ended";
    }
    if (snapshot.gateOpened) {
      return "The ancient gate stands open";
    }
    if (snapshot.victories >= snapshot.victoriesRequired) {
      return "The gate is waiting";
    }
    return "Follow the ancient road";
  }

  function enemyGlyph(name) {
    if (name === "Dragon") return "🐉";
    if (name === "Mage") return "✦";
    return "♞";
  }

  function setWidth(element, value, maximum) {
    const percent = maximum > 0 ? Math.max(0, Math.min(100, (value / maximum) * 100)) : 0;
    element.style.width = `${percent}%`;
  }

  function markChip(element, active, activeText, inactiveText) {
    element.classList.toggle("active", active);
    element.textContent = active ? activeText : inactiveText;
  }

  function renderProgress(snapshot) {
    const complete = [
      snapshot.victories >= snapshot.victoriesRequired,
      snapshot.hasKey || snapshot.gateOpened,
      snapshot.gateOpened,
      snapshot.phase === "victory"
    ];

    ui.progressSteps.forEach((step, index) => {
      step.classList.toggle("complete", complete[index]);
      const marker = step.querySelector("span");
      marker.textContent = complete[index] ? "✓" : String(index + 1);
    });
  }

  function render(snapshot) {
    currentState = snapshot;
    const isBattle = snapshot.phase === "battle";
    const isFinished = snapshot.phase === "victory" || snapshot.phase === "defeat";

    ui.sceneTitle.textContent = sceneTitleFor(snapshot);
    ui.narrative.textContent = snapshot.message;
    ui.phaseBadge.textContent = phaseLabel(snapshot.phase);
    ui.phaseBadge.className = `phase-badge ${snapshot.phase}`;

    ui.playerName.textContent = snapshot.playerName;
    ui.healthText.textContent = `${snapshot.health} / ${snapshot.maxHealth} HP`;
    setWidth(ui.healthBar, snapshot.health, snapshot.maxHealth);
    ui.goldStat.textContent = snapshot.gold;
    ui.potionStat.textContent = snapshot.potions;
    ui.victoryStat.textContent = `${snapshot.victories} / ${snapshot.victoriesRequired}`;

    markChip(ui.swordChip, snapshot.hasSword, "⚔ Sword equipped", "⚔ No sword");
    markChip(ui.keyChip, snapshot.hasKey, "🗝 Key secured", "🗝 No key");
    markChip(ui.gateChip, snapshot.gateOpened, "🚪 Gate open", "🚪 Gate sealed");

    ui.enemyPanel.classList.toggle("hidden", !isBattle);
    if (isBattle) {
      ui.enemyName.textContent = snapshot.enemyName;
      ui.enemyHealthText.textContent = `${snapshot.enemyHealth} / ${snapshot.enemyMaxHealth} HP`;
      setWidth(ui.enemyHealthBar, snapshot.enemyHealth, snapshot.enemyMaxHealth);
      ui.enemyGlyph.textContent = enemyGlyph(snapshot.enemyName);
    }

    ui.explorationActions.classList.toggle("hidden", isBattle);
    ui.battleActions.classList.toggle("hidden", !isBattle);
    ui.actionHeading.textContent = isBattle ? "Choose your move" : "Explore the road";

    $$("[data-action]").forEach((button) => {
      const action = button.dataset.action;
      let disabled = isFinished;

      if (isBattle) {
        disabled = disabled || !["attack", "heal", "run"].includes(action);
        if (action === "heal") disabled = disabled || snapshot.potions < 1;
      } else {
        disabled = disabled || ["attack", "heal", "run"].includes(action);
        if (action === "buy:potion") disabled = disabled || snapshot.gold < 3;
        if (action === "buy:sword") disabled = disabled || snapshot.gold < 8 || snapshot.hasSword;
        if (action === "buy:key") {
          disabled = disabled || snapshot.gold < 6 || snapshot.hasKey || snapshot.gateOpened;
        }
      }

      button.disabled = disabled;
    });

    renderProgress(snapshot);
  }

  function appendLog(message) {
    if (!message || message === lastMessage) return;
    lastMessage = message;

    const item = document.createElement("li");
    const time = document.createElement("time");
    time.textContent = new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
    item.append(time, document.createTextNode(message));
    ui.eventLog.prepend(item);

    while (ui.eventLog.children.length > 7) {
      ui.eventLog.lastElementChild.remove();
    }
  }

  function act(command) {
    if (!game) return;
    game.perform(command);
    const snapshot = state();
    appendLog(snapshot.message);
    render(snapshot);
  }

  function resetGame(name) {
    if (!game) return;
    const cleaned = name.trim() || "Adventurer";
    game.reset(cleaned);
    ui.eventLog.innerHTML = "";
    lastMessage = "";
    const snapshot = state();
    appendLog(snapshot.message);
    render(snapshot);
    ui.startScreen.classList.add("hidden");
  }

  function openStartScreen() {
    ui.startScreen.classList.remove("hidden");
    ui.nameInput.value = currentState?.playerName || "";
    setTimeout(() => ui.nameInput.focus(), 0);
  }

  function bindEvents() {
    $$("[data-action]").forEach((button) => {
      button.addEventListener("click", () => act(button.dataset.action));
    });

    ui.startForm.addEventListener("submit", (event) => {
      event.preventDefault();
      resetGame(ui.nameInput.value);
    });

    ui.newGameButton.addEventListener("click", openStartScreen);

    document.addEventListener("keydown", (event) => {
      if (ui.startScreen && !ui.startScreen.classList.contains("hidden")) return;
      if (event.target instanceof HTMLInputElement) return;

      const key = event.key.toLowerCase();
      const battleMap = { a: "attack", h: "heal", r: "run" };
      const exploreMap = { w: "walk", i: "inventory", b: "boss" };
      const command = currentState?.phase === "battle" ? battleMap[key] : exploreMap[key];

      if (command) {
        event.preventDefault();
        act(command);
      }
    });
  }

  async function boot() {
    bindEvents();

    try {
      wasm = await createMiniGameModule();
      game = new wasm.GameEngine("Adventurer");
      currentState = state();
      render(currentState);

      ui.runtimeBadge.textContent = "C++ engine ready";
      ui.runtimeBadge.classList.add("ready");
      ui.startButton.disabled = false;
      ui.startButton.textContent = "Begin adventure";
    } catch (error) {
      console.error(error);
      ui.runtimeBadge.textContent = "Engine failed to load";
      ui.sceneTitle.textContent = "Could not start MiniGame";
      ui.narrative.textContent =
        "The WebAssembly module did not load. Serve the web-dist directory over HTTP and try again.";
      ui.startButton.textContent = "Engine unavailable";
    }
  }

  boot();
})();
