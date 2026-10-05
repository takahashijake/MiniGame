# Architecture

MiniGame has one C++ domain engine and two presentation adapters.

## GameEngine

GameEngine owns:

- Player and equipment state;
- Progression and boss-gate state;
- active enemy encounter;
- game phase;
- run seed;
- RandomSource;
- latest event message; and
- persistence serialization/restoration.

Each frontend sends a semantic command such as walk, attack, heal, run, buy:shield, or boss. The engine performs one bounded transition and exposes a GameSnapshot.

## Persistence boundary

Production randomness uses std::mt19937. RandomGenerator can serialize and restore its internal state.

GameEngine save data includes the original seed plus the live RNG state, player health/inventory, progression, phase, active enemy/HP, boss-battle state, and message. That means loading resumes the same future random sequence.

The browser stores the opaque save string in localStorage. The CLI writes the same engine save format to .minigame-save.

## Frontends

### Native

GameState translates terminal commands into GameEngine actions and renders GameSnapshot. It also provides file save/load and optional seeded startup.

### Browser

web_bindings.cpp exposes GameEngine through Emscripten/Embind.

app.js renders state, handles controls and keyboard shortcuts, stores autosaves, restores Continue Run, and registers the offline service worker. It does not implement gameplay rules.

## Web delivery

The Emscripten build emits minigame.js and minigame.wasm beside the static frontend.

A service worker caches the application shell for offline replay after the first successful load.

The Deploy Web Product workflow builds that bundle from main and deploys web-dist through GitHub Pages.

## Dependency direction

~~~text
Player / Character / Progression / RandomSource
                    ↓
                GameEngine
               ↙          ↘
        Native GameState   Emscripten bindings
                                ↓
                          Browser frontend
~~~

Tests link directly against the engine/core and inject deterministic randomness where needed.
