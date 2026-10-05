# Architecture

MiniGame is organized around one reusable C++ gameplay state machine with two presentation adapters: a native CLI and a browser/WebAssembly frontend.

## Core components

- **GameEngine** is the product-facing domain API. It owns the current Player, Progression, active Character encounter, RandomSource, phase, and latest event message. Every user action advances the engine by at most one gameplay turn.
- **Player** owns health, inventory, Potion consumption, and player attack damage.
- **Character** is the polymorphic enemy base. Knight, Mage, and Dragon define distinct health, damage, and healing behavior.
- **Progression** owns normal-victory count, merchant pricing, the boss gate, and final-boss completion.
- **RandomSource** is the randomness boundary. RandomGenerator is the production implementation; tests inject deterministic sequences.
- **GameState** is now only a native terminal adapter. It translates keyboard/menu input into GameEngine commands and renders GameSnapshot values.
- **web_bindings.cpp** exposes GameEngine to JavaScript through Emscripten Embind.
- **web/app.js** contains view state and DOM rendering only; it does not implement combat or progression rules.

## Why the engine changed

The earlier BattleSequence owned an entire blocking stdin/stdout battle. That worked for a terminal prototype but could not support a GUI/browser client without duplicating gameplay.

GameEngine changes the interaction contract from:

~~~text
start battle -> block until battle finishes -> return
~~~

to:

~~~text
frontend action -> one engine transition -> snapshot -> render
~~~

That makes the C++ core usable from a terminal, browser, test harness, or future desktop GUI.

## State flow

The engine has four externally visible phases:

1. **exploring** — walk, inspect inventory, buy items, or challenge the gate;
2. **battle** — attack, heal, or run;
3. **defeat** — terminal loss state;
4. **victory** — terminal win state.

A GameSnapshot contains all presentation-safe state needed by either frontend. The web bridge additionally serializes the snapshot to JSON for JavaScript.

## Browser boundary

The Emscripten target creates a `GameEngine` class in JavaScript with three operations:

- `perform(command)`
- `reset(playerName)`
- `stateJson()`

The JavaScript frontend dispatches semantic commands such as `walk`, `attack`, `buy:sword`, and `boss`. It then renders the returned state. There is no duplicated damage, loot, merchant, or boss-gate logic in JavaScript.

## Build graph

Native:

~~~text
minigame_core -> minigame (main.cpp + game_state.cpp)
             -> minigame_tests
~~~

Web:

~~~text
minigame_core -> minigame_web (web_bindings.cpp)
             -> minigame.js + minigame.wasm
             + web static assets -> web-dist/
~~~
