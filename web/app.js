(() => {
  "use strict";

  const SAVE_KEY = "minigame.v1.2.save";
  const $ = (selector) => document.querySelector(selector);
  const $$ = (selector) => [...document.querySelectorAll(selector)];

  const ui = {
    runtimeBadge: $("#runtimeBadge"),
    saveBadge: $("#saveBadge"),
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
    seedStat: $("#seedStat"),
    swordChip: $("#swordChip"),
    shieldChip: $("#shieldChip"),
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
    continueButton: $("#continueButton"),
    nameInput: $("#nameInput"),
    seedInput: $("#seedInput"),
    progressSteps: $$("#progressSteps .progress-step")
  };

  let wasm = null;
  let game = null;
  let lastMessage = "";
  let currentState = null;

  function readState() {
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
      if (snapshot.enemyName === "Dragon" && snapshot.enemyEnraged) return "The Dragon is enraged";
      if (snapshot.enemyName === "Dragon") return "The Dragon has arrived";
      return `${snapshot.enemyName} encounter`;
    }
    if (snapshot.phase === "victory") return "The road is yours";
    if (snapshot.phase === "defeat") return "Your run has ended";
    if (snapshot.gateOpened) return "The ancient gate stands open";
    if (snapshot.victories >= snapshot.victoriesRequired) return "The gate is waiting";
    return "Follow the ancient road";
  }

  function enemyGlyph(name) {
    if (name === "Dragon") return "🐉";
    if (name === "Mage") return "✦";
    if (name === "Rogue") return "🗡";
    if (name === "Golem") return "⬢";
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

    renderPlayer(snapshot);
    renderEnemy(snapshot, isBattle);
    renderActions(snapshot, isBattle, isFinished);

    renderProgress(snapshot);
  }

  function renderPlayer(snapshot) {
    ui.playerName.textContent = snapshot.playerName;
    ui.healthText.textContent =
      `${snapshot.health} / ${snapshot.maxHealth} HP${snapshot.defense > 0 ? ` · ${snapshot.defense} DEF` : ""}`;
    setWidth(ui.healthBar, snapshot.health, snapshot.maxHealth);
    ui.goldStat.textContent = snapshot.gold;
    ui.potionStat.textContent = snapshot.potions;
    ui.victoryStat.textContent = `${snapshot.victories} / ${snapshot.victoriesRequired}`;
    ui.seedStat.textContent = String(snapshot.seed);

    markChip(ui.swordChip, snapshot.hasSword, "⚔ Sword equipped", "⚔ No sword");
    markChip(
      ui.shieldChip,
      snapshot.hasShield,
      `◈ Shield equipped (-${snapshot.defense})`,
      "◈ No shield"
    );
    markChip(ui.keyChip, snapshot.hasKey, "🗝 Key secured", "🗝 No key");
    markChip(ui.gateChip, snapshot.gateOpened, "🚪 Gate open", "🚪 Gate sealed");

  }

  function renderEnemy(snapshot, isBattle) {
    ui.enemyPanel.classList.toggle("hidden", !isBattle);
    ui.enemyPanel.classList.toggle("enraged", Boolean(snapshot.enemyEnraged));
    if (isBattle) {
      ui.enemyName.textContent =
        snapshot.enemyName + (snapshot.enemyEnraged ? " · ENRAGED" : "");
      ui.enemyHealthText.textContent =
        `${snapshot.enemyHealth} / ${snapshot.enemyMaxHealth} HP`;
      setWidth(ui.enemyHealthBar, snapshot.enemyHealth, snapshot.enemyMaxHealth);
      ui.enemyGlyph.textContent = enemyGlyph(snapshot.enemyName);
    }

  }

  function renderActions(snapshot, isBattle, isFinished) {
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
        if (action === "buy:shield") {
          disabled = disabled || snapshot.gold < 10 || snapshot.hasShield;
        }
        if (action === "buy:key") {
          disabled = disabled || snapshot.gold < 6 || snapshot.hasKey || snapshot.gateOpened;
        }
      }

      if (isBattle && snapshot.enemyName === "Dragon" && action === "run") {
        disabled = false;
      }

      button.disabled = disabled;
    });

  }

  function appendLog(message) {
    if (!message || message === lastMessage) return;
    lastMessage = message;

    const item = document.createElement("li");
    const time = document.createElement("time");
    time.textContent = new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
    item.append(time, document.createTextNode(message));
    ui.eventLog.prepend(item);

    while (ui.eventLog.children.length > 8) {
      ui.eventLog.lastElementChild.remove();
    }
  }

  function persist() {
    if (!game) return;
    try {
      localStorage.setItem(SAVE_KEY, game.saveState());
      ui.saveBadge.textContent = "Autosaved";
      ui.saveBadge.classList.add("saved");
      ui.continueButton.disabled = false;
    } catch (error) {
      console.warn("MiniGame autosave failed", error);
      ui.saveBadge.textContent = "Autosave unavailable";
      ui.saveBadge.classList.remove("saved");
    }
  }

  function act(command) {
    if (!game) return;
    game.perform(command);
    const snapshot = readState();
    appendLog(snapshot.message);
    render(snapshot);
    persist();
  }

  function parseSeed() {
    const raw = ui.seedInput.value.trim();
    if (!raw) return null;
    const value = Number(raw);
    if (!Number.isInteger(value) || value < 0 || value > 4294967295) return null;
    return value;
  }

  function resetGame(name) {
    if (!game) return;
    const cleaned = name.trim() || "Adventurer";
    const rawSeed = ui.seedInput.value.trim();
    const seed = parseSeed();

    if (rawSeed && seed === null) {
      ui.seedInput.setCustomValidity("Use an integer seed from 0 to 4294967295.");
      ui.seedInput.reportValidity();
      return;
    }

    ui.seedInput.setCustomValidity("");
    if (seed === null) {
      game.reset(cleaned);
    } else {
      game.resetSeeded(cleaned, seed);
    }

    ui.eventLog.innerHTML = "";
    lastMessage = "";
    const snapshot = readState();
    appendLog(snapshot.message);
    render(snapshot);
    persist();
    ui.startScreen.classList.add("hidden");
  }

  function readSavedRun() {
    try {
      return localStorage.getItem(SAVE_KEY);
    } catch (error) {
      console.warn("MiniGame save storage unavailable", error);
      ui.saveBadge.textContent = "Autosave unavailable";
      ui.saveBadge.classList.remove("saved");
      ui.continueButton.disabled = true;
      return null;
    }
  }

  function continueSavedRun() {
    const save = readSavedRun();
    if (!save || !game) return;

    if (!game.loadState(save)) {
      try {
        localStorage.removeItem(SAVE_KEY);
      } catch (error) {
        console.warn("MiniGame invalid save could not be removed", error);
      }
      ui.continueButton.disabled = true;
      ui.saveBadge.textContent = "Saved run was invalid";
      return;
    }

    ui.eventLog.innerHTML = "";
    lastMessage = "";
    const snapshot = readState();
    appendLog("Saved run restored.");
    appendLog(snapshot.message);
    render(snapshot);
    ui.startScreen.classList.add("hidden");
    ui.saveBadge.textContent = "Save restored";
    ui.saveBadge.classList.add("saved");
  }

  function openStartScreen() {
    ui.startScreen.classList.remove("hidden");
    ui.nameInput.value = currentState?.playerName || "";
    ui.seedInput.value = "";
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

    ui.seedInput.addEventListener("input", () => ui.seedInput.setCustomValidity(""));

    ui.continueButton.addEventListener("click", continueSavedRun);
    ui.newGameButton.addEventListener("click", openStartScreen);

    document.addEventListener("keydown", (event) => {
      if (!ui.startScreen.classList.contains("hidden")) return;
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
      currentState = readState();
      render(currentState);

      ui.runtimeBadge.textContent = "C++ engine ready";
      ui.runtimeBadge.classList.add("ready");
      ui.startButton.disabled = false;
      ui.startButton.textContent = "Begin adventure";

      const hasSave = Boolean(readSavedRun());
      ui.continueButton.disabled = !hasSave;
      if (ui.saveBadge.textContent !== "Autosave unavailable") {
        ui.saveBadge.textContent = hasSave ? "Saved run found" : "Autosave ready";
      }
      ui.saveBadge.classList.toggle("saved", hasSave);
    } catch (error) {
      console.error(error);
      ui.runtimeBadge.textContent = "Engine failed to load";
      ui.sceneTitle.textContent = "Could not start MiniGame";
      ui.narrative.textContent =
        "The WebAssembly module did not load. Serve the web-dist directory over HTTP and try again.";
      ui.startButton.textContent = "Engine unavailable";
      ui.continueButton.disabled = true;
    }
  }

  if ("serviceWorker" in navigator) {
    navigator.serviceWorker.register("./sw.js").catch((error) => {
      console.warn("MiniGame service worker registration failed", error);
    });
  }

  boot();
})();
