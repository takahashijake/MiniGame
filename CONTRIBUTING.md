# Contributing

MiniGame is intentionally compact, but changes should preserve the separation between gameplay rules and presentation.

## Native verification

~~~sh
cmake -S . -B build -DMINIGAME_BUILD_TESTS=ON -DMINIGAME_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
~~~

For sanitizers on GCC or Clang:

~~~sh
cmake -S . -B build-sanitize -DMINIGAME_BUILD_TESTS=ON -DMINIGAME_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
~~~

## Web verification

With Emscripten installed:

~~~sh
node --check web/app.js
node --check web/sw.js
node tests/test_web.cjs
node tests/test_sw.cjs
emcmake cmake -S . -B build-web -DMINIGAME_BUILD_TESTS=OFF -DMINIGAME_WARNINGS_AS_ERRORS=ON
cmake --build build-web --target minigame_web
node tests/test_web.cjs build-web/web-dist/minigame.js ./build/minigame_replay
python3 -m http.server 8080 --directory build-web/web-dist
~~~

Exercise exploration, normal combat, merchant disabled/enabled states, healing, running, gate feedback, and a new-run reset in the browser.

## Design guidelines

- Put gameplay rules in GameEngine, Player, Character, or Progression—not JavaScript or terminal rendering.
- Inject RandomSource when behavior depends on randomness so core tests remain deterministic.
- Keep each GameEngine command to a bounded state transition suitable for interactive frontends.
- Prefer value ownership, references, and standard smart pointers over owning raw pointers.
- Add regression coverage for bug fixes and focused tests for new mechanics.
- Keep frontend controls accessible and usable at desktop and mobile widths.
- Update CHANGELOG.md and relevant docs when behavior or build steps change.

See [QA coverage and browser smoke checks](docs/QA.md) for persistence and seeded replay verification.
