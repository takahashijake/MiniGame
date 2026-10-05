# Contributing

MiniGame is intentionally small, but changes should keep the game loop easy to understand and test.

## Local verification

~~~sh
cmake -S . -B build -DMINIGAME_BUILD_TESTS=ON -DMINIGAME_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
~~~

For a sanitizer pass on GCC or Clang:

~~~sh
cmake -S . -B build-sanitize -DMINIGAME_BUILD_TESTS=ON -DMINIGAME_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
~~~

## Design guidelines

- Keep game rules in the core types instead of burying them in terminal rendering.
- Inject RandomSource when behavior depends on randomness so tests can remain deterministic.
- Prefer value ownership, references, and standard smart pointers over owning raw pointers.
- Add a regression test for bug fixes and a focused test for new mechanics.
- Keep player-facing commands discoverable from the main menu.
- Update CHANGELOG.md when behavior changes.
