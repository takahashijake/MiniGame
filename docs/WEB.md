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
