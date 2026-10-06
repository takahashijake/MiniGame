// Dependency-free adapter regression tests; optionally run against the built WASM engine.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { execFileSync } = require('node:child_process');

class Element {
  constructor() {
    this.textContent = ''; this.value = ''; this.disabled = false;
    this.style = {}; this.dataset = {}; this.children = []; this.listeners = {};
    this.classes = new Set();
    this.classList = {
      add: (name) => this.classes.add(name), remove: (name) => this.classes.delete(name),
      contains: (name) => this.classes.has(name),
      toggle: (name, active) => active ? this.classes.add(name) : this.classes.delete(name)
    };
  }
  addEventListener(name, handler) { this.listeners[name] = handler; }
  fire(name) { this.listeners[name]?.({ preventDefault() {} }); }
  setCustomValidity(message) { this.validationMessage = message; }
  reportValidity() {}
  focus() {}
  querySelector() { return new Element(); }
  append(...items) { this.children.push(...items); }
  prepend(item) { this.children.unshift(item); }
  set innerHTML(value) { this.children = []; }
  get lastElementChild() { return { remove: () => this.children.pop() }; }
}

class FakeEngine {
  constructor() { this.resetSeeded('Adventurer', 42); }
  reset(name) { this.resetSeeded(name, 42); }
  resetSeeded(playerName, seed) {
    this.state = { playerName, seed, phase: 'exploring', health: 100, maxHealth: 100,
      gold: 0, potions: 0, defense: 0, victories: 0, victoriesRequired: 3,
      enemyName: '', message: 'New run' };
  }
  stateJson() { return JSON.stringify(this.state); }
  saveState() { return this.stateJson(); }
  loadState(save) { try { this.state = JSON.parse(save); return true; } catch { return false; } }
  perform(command) { this.state.message = command; }
}

async function adapter(factory, storage = new Map(), blocked = false) {
  const elements = new Map();
  const get = (selector) => {
    if (!elements.has(selector)) elements.set(selector, new Element());
    return elements.get(selector);
  };
  const actions = ['walk', 'attack', 'heal', 'run', 'boss', 'inventory',
    'buy:potion', 'buy:sword', 'buy:shield', 'buy:key'].map((action) => {
    const element = new Element(); element.dataset.action = action; return element;
  });
  let engine;
  const mod = await factory();
  const context = {
    document: { querySelector: get, querySelectorAll: (selector) =>
      selector === '[data-action]' ? actions : [], createElement: () => new Element(),
      createTextNode: (text) => text, addEventListener() {} },
    localStorage: {
      getItem: (key) => { if (blocked) throw Error('blocked'); return storage.get(key) || null; },
      setItem: (key, value) => { if (blocked) throw Error('blocked'); storage.set(key, value); },
      removeItem: (key) => storage.delete(key)
    },
    createMiniGameModule: async () => ({ GameEngine: function(name) {
      engine = new mod.GameEngine(name); return engine;
    } }),
    navigator: {}, HTMLInputElement: Element, setTimeout, Date,
    console: { warn() {}, error: (...args) => { throw Error(args.join(' ')); } }
  };
  vm.runInNewContext(fs.readFileSync('web/app.js', 'utf8'), context);
  await new Promise(setImmediate);
  return { get, storage, engine, action: (name) => actions.find((a) => a.dataset.action === name) };
}

async function runAdapterTests(factory) {
  const key = 'minigame.v1.2.save';
  const ui = await adapter(factory);
  ui.get('#nameInput').value = 'Tester';
  ui.get('#seedInput').value = '4294967296';
  ui.get('#startForm').fire('submit');
  assert.match(ui.get('#seedInput').validationMessage, /integer seed/);
  assert.equal(ui.storage.has(key), false);
  ui.get('#seedInput').value = '42';
  ui.get('#seedInput').fire('input');
  assert.equal(ui.get('#seedInput').validationMessage, '');
  ui.get('#startForm').fire('submit');
  assert.equal(ui.get('#seedStat').textContent, '42');
  assert.equal(ui.get('#startScreen').classList.contains('hidden'), true);
  ui.action('walk').fire('click');
  const saved = ui.storage.get(key);
  const state = ui.engine.stateJson();
  const reload = await adapter(factory, ui.storage);
  assert.equal(reload.get('#continueButton').disabled, false);
  reload.get('#continueButton').fire('click');
  assert.equal(reload.engine.stateJson(), state);
  assert.equal(reload.storage.get(key), saved);
  ui.get('#seedInput').value = '0';
  ui.get('#startForm').fire('submit');
  assert.equal(ui.get('#seedStat').textContent, '0');
  assert.equal(ui.get('#eventLog').children.length, 1);

  const corrupt = await adapter(factory, new Map([[key, 'not a save']]));
  const before = corrupt.engine.stateJson();
  corrupt.get('#continueButton').fire('click');
  assert.equal(corrupt.engine.stateJson(), before);
  assert.equal(corrupt.storage.has(key), false);
  assert.equal(corrupt.get('#continueButton').disabled, true);

  const blocked = await adapter(factory, new Map(), true);
  assert.equal(blocked.get('#runtimeBadge').textContent, 'C++ engine ready');
  assert.equal(blocked.get('#startButton').disabled, false);
  assert.equal(blocked.get('#saveBadge').textContent, 'Autosave unavailable');
  blocked.get('#startForm').fire('submit');
  blocked.action('walk').fire('click');
  assert.equal(blocked.get('#startScreen').classList.contains('hidden'), true);
  for (const instance of [ui, reload, corrupt, blocked]) instance.engine.delete?.();
}

async function wasmRegression(modulePath, nativePath) {
  const create = require(path.resolve(modulePath));
  const wasmBinary = fs.readFileSync(modulePath.replace(/\.js$/, '.wasm'));
  const mod = await create({ wasmBinary });
  const factory = async () => mod;
  await runAdapterTests(factory);
  const game = new mod.GameEngine('Replay', 42);
  const native = execFileSync(nativePath, { encoding: 'utf8' }).trim().split('\n');
  for (let turn = 0; turn < native.length; ++turn) {
    assert.equal(game.stateJson(), native[turn], `native/WASM mismatch at turn ${turn}`);
    const copy = new mod.GameEngine('Other', 0);
    assert.equal(copy.loadState(game.saveState()), true);
    const command = JSON.parse(game.stateJson()).phase === 'battle' ? 'attack' : 'walk';
    game.perform(command); copy.perform(command);
    assert.equal(game.saveState(), copy.saveState(), 'WASM restore lost RNG continuity');
    copy.delete();
  }
  // Restore a boss encounter through the real C++ boundary and check presentation.
  const rng = game.saveState().match(/"([^"\n]*)"$/)[1];
  const save = `MG2 42 "Boss" 100 0 2 1 1 0 3 1 0 1 1 "Dragon" 90 "Boss fixture" "${rng}"`;
  const ui = await adapter(factory, new Map([['minigame.v1.2.save', save]]));
  ui.get('#continueButton').fire('click');
  assert.equal(ui.get('#sceneTitle').textContent, 'The Dragon is enraged');
  assert.equal(ui.get('#enemyPanel').classList.contains('enraged'), true);
  assert.match(ui.get('#healthText').textContent, /5 DEF/);
  ui.action('run').fire('click');
  assert.equal(JSON.parse(ui.engine.stateJson()).phase, 'battle');
  assert.match(ui.get('#narrative').textContent, /no escape/);
  game.delete(); ui.engine.delete();
}

(async () => {
  await runAdapterTests(async () => ({ GameEngine: FakeEngine }));
  if (process.argv[2]) await wasmRegression(process.argv[2], process.argv[3]);
  console.log('Browser adapter regressions passed' + (process.argv[2] ? ' with real WASM/native replay parity.' : '.'));
})().catch((error) => { console.error(error); process.exitCode = 1; });
