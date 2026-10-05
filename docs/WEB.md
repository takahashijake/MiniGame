# Web Product

The MiniGame browser edition is a static application powered by the C++ core compiled to WebAssembly.

## Build

With Emscripten installed:

~~~sh
emcmake cmake -S . -B build-web -DMINIGAME_BUILD_TESTS=OFF
cmake --build build-web --target minigame_web
~~~

The build copies the frontend assets and generated runtime into `build-web/web-dist`.

Serve it locally:

~~~sh
python3 -m http.server 8080 --directory build-web/web-dist
~~~

## Frontend responsibilities

The browser layer is intentionally thin. It handles:

- button and keyboard input;
- responsive rendering;
- health/progression bars;
- inventory and merchant presentation;
- event-log history; and
- new-run UI.

It does **not** calculate damage, decide loot, mutate inventory, price merchant items, select enemies, or decide whether the boss is unlocked. Those rules live in C++.

## JavaScript-to-C++ contract

Emscripten exposes `GameEngine` through Embind.

~~~js
const wasm = await createMiniGameModule();
const game = new wasm.GameEngine("Ada");

game.perform("walk");
const state = JSON.parse(game.stateJson());
~~~

Supported semantic commands include:

- `walk`
- `inventory`
- `boss`
- `attack`
- `heal`
- `run`
- `buy:potion`
- `buy:sword`
- `buy:key`

## Deployment

The contents of `web-dist` are ordinary static assets and can be hosted on GitHub Pages, Cloudflare Pages, Netlify, S3/CloudFront, or any static web server that serves `.wasm` files.

No backend service or database is required for the current single-player session model. Game state lives in WebAssembly memory for the life of the browser tab.

## Future extensions

The GameEngine boundary makes several future features straightforward without changing frontend ownership of rules:

- save/load serialization;
- multiple encounter types;
- equipment slots;
- achievements;
- richer animation/audio;
- deterministic seeds and shareable runs; and
- an Electron/Tauri desktop wrapper around the same web bundle.
