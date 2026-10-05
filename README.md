# MiniGame

MiniGame is a compact turn-based terminal RPG written in modern C++. The project started as a fundamentals exercise and now serves as a small, testable example of clean C++ architecture, deterministic game logic, persistence, CI, and release automation.

## Highlights

- Polished terminal UI with health bars, color, dashboard, inventory, and battle views.
- Campaign progression with XP, levels, gold, scaled enemies, boss milestones, potions, and bombs.
- Save/load support with validation and a versioned save format.
- Testable combat and campaign core separated from terminal I/O.
- CMake build on Linux, macOS, and Windows.
- Automated unit tests, warning-as-error builds, sanitizers, formatting checks, and tagged releases.
- No third-party runtime dependencies.

## Build and run

Requirements: a C++17 compiler and CMake 3.16+.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/minigame
```

On Windows, run `build\\bin\\minigame.exe` after building. A convenience Makefile is also available on Unix-like systems:

```bash
make run
```

## Controls

World: `E` explore, `I` inventory, `S` save, `L` load, `Q` quit.

Battle: `A` attack, `D` defend, `H` heal, `B` bomb, `R` run. Boss encounters cannot be escaped.

Set `NO_COLOR=1` to disable ANSI colors, or `MINIGAME_NO_CLEAR=1` to disable screen clearing for logs and accessibility workflows.

## Test

```bash
cmake -S . -B build -DMINIGAME_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

For a sanitizer build on GCC or Clang:

```bash
cmake -S . -B build-asan -DMINIGAME_ENABLE_SANITIZERS=ON
cmake --build build-asan --parallel
ctest --test-dir build-asan --output-on-failure
```

## Project layout

```text
include/minigame/   Public game-core interfaces
src/                Game core, terminal UI, and executable entry point
tests/              Dependency-free unit/integration tests
docs/               Architecture notes
.github/workflows/  CI and tagged-release automation
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for design details and [CHANGELOG.md](CHANGELOG.md) for release history.
