# Web Product

The browser edition is a static application powered by the shared C++ GameEngine compiled to WebAssembly.

## Build

~~~sh
emcmake cmake -S . -B build-web -DMINIGAME_BUILD_TESTS=OFF
cmake --build build-web --target minigame_web
python3 -m http.server 8080 --directory build-web/web-dist
~~~

## WebAssembly API

The Emscripten bridge exposes:

- GameEngine(playerName)
- GameEngine(playerName, seed)
- perform(command)
- reset(playerName)
- resetSeeded(playerName, seed)
- stateJson()
- saveState()
- loadState(save)

JavaScript owns DOM rendering, controls, localStorage, event history, and run setup. Damage, defense, enemy selection, enemy AI, loot, merchant pricing, boss enrage, progression, save structure, and RNG state remain in C++.

## Autosave

After each engine action, the browser stores the opaque C++ save blob under a versioned localStorage key. Reloading enables Continue saved run.

The save includes serialized std::mt19937 state, so the next random roll is preserved.

## Seeded runs

The new-run dialog accepts an optional seed from 0 through 4294967295. The active seed is displayed in the HUD.

## Deployment

The generated web-dist directory is static and can be hosted by GitHub Pages, Cloudflare Pages, Netlify, S3/CloudFront, or any server that serves WebAssembly.

## Storage and run lifecycle

- Save storage is local to the browser origin and profile. Native saves use the current
  working directory. There is no account synchronization or cloud save.
- Continue restores exploration, live combat, or a completed victory/defeat. A completed
  browser run remains terminal until a new run is started. The CLI exits on completion;
  save with `S` before exiting while a run is active.
- Starting a new run replaces the existing autosave. Closing the new-run dialog by
  continuing the saved run restores the previous run instead.
- Corrupt saves are rejected without changing the engine. The browser removes the
  invalid autosave and disables Continue. The CLI keeps the invalid file and reports it.
- If storage access or writes are denied, gameplay still works; the save badge reports
  Autosave unavailable. Reloading cannot recover an unsaved session.
- Event history is session-only presentation state; it is rebuilt from the restored
  engine message rather than stored as part of the save.

## Offline shell

The service worker caches only allowlisted same-origin app assets after validating
successful responses and content types. HTTPS and localhost development are supported.
Activation removes only old `minigame-` caches, preserving other applications' caches.
A successful online load is required before offline play; saved progress still uses
localStorage. A new frontend/engine release must use a new service-worker cache name
so cached JavaScript and WASM remain an aligned bundle.

## Regression verification

See [QA.md](QA.md) for native, WASM, adapter and service-worker test commands.
The dependency-free adapter harness exercises the actual compiled WASM engine when
its module and native replay executable are supplied. It checks storage denial,
corrupt Continue, seed correction, new-run reset, resumed combat, Dragon rendering,
and native/WASM snapshot parity. It simulates DOM elements; visual layout and real
browser/service-worker installation still need the manual smoke checks in QA.md.
