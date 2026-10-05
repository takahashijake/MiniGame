# MiniGame

MiniGame is a turn-based C++ adventure shipped as both a native terminal application and a browser game. The browser edition runs the same C++ gameplay engine through WebAssembly rather than reimplementing game rules in JavaScript.

## v1.2

The project now includes:

- persistent runs: browser autosave/Continue and native save/load;
- full RNG continuity: saves preserve the live std::mt19937 state;
- optional 32-bit seeded runs for reproducible encounters and rolls;
- Knight, Mage, Rogue, and Golem normal enemies;
- Sword offense and Shield damage mitigation;
- combat, loot, healing shrine, and quiet-road exploration events;
- a 180 HP Dragon that cannot be fled and enrages below half health;
- shared C++ GameEngine rules across CLI and browser;
- Linux/macOS/Windows tests, sanitizers, and a real Emscripten build.

## Browser build

Install Emscripten, then:

~~~sh
emcmake cmake -S . -B build-web -DMINIGAME_BUILD_TESTS=OFF
cmake --build build-web --target minigame_web
python3 -m http.server 8080 --directory build-web/web-dist
~~~

Open http://localhost:8080.

The browser frontend exposes combat, exploration, merchant controls, equipment/progression state, run seed, autosave status, and Continue saved run. All gameplay decisions remain in C++.

## Native build

~~~sh
cmake -S . -B build -DMINIGAME_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/minigame
~~~

The CLI asks for an optional seed. During exploration, S saves and L loads .minigame-save; combat can also be saved.

## Gameplay

Win three normal battles, secure a Key, open the gate, and defeat the Dragon.

| Enemy | Role |
| --- | --- |
| Knight | balanced fighter |
| Mage | lighter health, stronger healing |
| Rogue | 65 HP glass cannon |
| Golem | 125 HP tank |
| Dragon | 180 HP boss; enrages below half health |

| Item | Effect | Price |
| --- | --- | ---: |
| Potion | restore 25–45 HP | 3 Gold |
| Key | opens boss gate | 6 Gold |
| Sword | +8 attack damage | 8 Gold |
| Shield | -5 incoming damage | 10 Gold |

## Deterministic runs and saves

Every production run has a visible 32-bit seed. The same seed plus the same commands yields the same RNG sequence.

The save format stores player state, inventory, progression, encounter state, phase, message, original seed, and the live RNG engine state. Loading therefore resumes the next random transition exactly rather than restarting the sequence.

The browser stores this C++ save blob in localStorage. The CLI stores the same engine state in .minigame-save.

## Architecture

~~~text
include/minigame/
  game_engine.h      state machine + persistence API
  player.h           combat, equipment, inventory
  character.h        enemy hierarchy
  progression.h      rewards, merchant, gate
  random.h           serializable RNG abstraction

src/
  game_engine.cpp    gameplay, encounters, saves, seeds
  game_state.cpp     terminal adapter + file persistence
  web_bindings.cpp   Emscripten bridge

web/
  index.html
  styles.css
  app.js             UI + local autosave

tests/
  test_core.cpp
~~~

See docs/ARCHITECTURE.md, docs/GAMEPLAY.md, and docs/WEB.md.
