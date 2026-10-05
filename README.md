# MiniGame

[![CI](https://github.com/takahashijake/MiniGame/actions/workflows/ci.yml/badge.svg)](https://github.com/takahashijake/MiniGame/actions/workflows/ci.yml)

MiniGame is a turn-based C++ adventure that now ships as both a native terminal game and a browser product. The browser edition is not a JavaScript rewrite: the same C++ gameplay engine is compiled to WebAssembly with Emscripten and driven by a responsive HTML/CSS/JavaScript frontend.

## Product surfaces

### Browser edition

The web client provides a full dashboard for:

- exploration and random encounters;
- live player/enemy health;
- turn-by-turn combat;
- inventory and progression state;
- merchant purchases;
- boss-gate progress;
- recent event history;
- keyboard shortcuts; and
- new-run/reset flow.

All gameplay state transitions happen inside `minigame::GameEngine`, compiled from C++ to `minigame.wasm`.

### Native CLI

The terminal application remains supported and now uses the exact same turn-by-turn `GameEngine` as the web client. This keeps combat, economy, progression, and win/loss rules consistent across both frontends.

## Gameplay

Explore the road, defeat Knights and Mages, collect loot, earn Gold, and use the travelling merchant to prepare for the final encounter.

To complete a run:

1. win three normal battles;
2. obtain a Key by exploration or purchase;
3. open the ancient boss gate; and
4. defeat the Dragon.

Items have concrete roles:

- **Potion** restores 25–45 HP.
- **Sword** permanently adds 8 attack damage.
- **Gold** buys equipment at the travelling merchant.
- **Key** opens the Dragon gate.

## Build the native application

Requirements: CMake 3.16+ and a C++17 compiler.

~~~sh
cmake -S . -B build -DMINIGAME_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/minigame
~~~

Or:

~~~sh
make test
make run
~~~

## Build the browser application

Install the Emscripten SDK so `emcmake` and `em++` are available, then run:

~~~sh
emcmake cmake -S . -B build-web -DMINIGAME_BUILD_TESTS=OFF
cmake --build build-web --target minigame_web
~~~

The complete static product is emitted to:

~~~text
build-web/web-dist/
├── index.html
├── styles.css
├── app.js
├── minigame.js
└── minigame.wasm
~~~

Serve that directory over HTTP. For example:

~~~sh
python3 -m http.server 8080 --directory build-web/web-dist
~~~

Then open `http://localhost:8080`.

The browser must be served over HTTP rather than opened directly from `file://` because the WebAssembly runtime is fetched as a separate asset.

## Architecture

~~~text
include/minigame/
  game_engine.h      shared state-machine API
  player.h           player health and inventory
  character.h        Knight, Mage, Dragon hierarchy
  progression.h      rewards, merchant, boss gate
  random.h           production/test randomness seam

src/
  game_engine.cpp    gameplay orchestration used by every frontend
  game_state.cpp     native terminal adapter
  web_bindings.cpp   Emscripten/Embind adapter
  ...

web/
  index.html         browser application shell
  styles.css         responsive product UI
  app.js             rendering and user interaction

tests/
  test_core.cpp      deterministic engine and regression tests
~~~

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the state-flow design and [docs/WEB.md](docs/WEB.md) for the WebAssembly/frontend contract.

## Verification

CI runs:

- warning-as-error native builds/tests on Ubuntu, macOS, and Windows;
- AddressSanitizer + UndefinedBehaviorSanitizer tests on Linux;
- JavaScript syntax validation;
- an Emscripten WebAssembly build; and
- bundle assertions for the generated browser product.

Tagged releases package all three native targets plus the browser bundle.

## History

The project began in July 2025 as a C++ fundamentals exercise. Version 1.0 modernized the repository and added a complete adventure progression loop. Version 1.1 turns that engine into a concrete multi-frontend product while retaining C++ as the single source of gameplay truth.
