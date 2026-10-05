# MiniGame

[![CI](https://github.com/takahashijake/MiniGame/actions/workflows/ci.yml/badge.svg)](https://github.com/takahashijake/MiniGame/actions/workflows/ci.yml)

MiniGame is a small turn-based terminal adventure written in modern C++. It started as a fundamentals project for practicing polymorphism, encapsulation, header boundaries, and randomized combat. The current version keeps that original spirit while giving the repository a production-style structure, automated tests, cross-platform CI, and a complete game loop.

## Gameplay

You explore by walking, fight Knights and Mages, collect loot, earn Gold, and use a travelling merchant to prepare for the final encounter. The goal is to:

1. win three normal battles;
2. obtain a Key by exploration or purchase;
3. open the ancient boss gate; and
4. defeat the Dragon.

The original inventory items now have concrete roles: Potions restore HP, the Sword permanently improves attack damage, Gold powers the merchant economy, and the Key unlocks the boss gate.

## Build and run

Requirements: a C++17 compiler and CMake 3.16 or newer.

~~~sh
cmake -S . -B build -DMINIGAME_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/minigame
~~~

On multi-config generators such as Visual Studio, run the executable from the generated configuration directory (for example build/Debug/minigame.exe).

A convenience Makefile is also included:

~~~sh
make test
make run
~~~

## Repository layout

~~~text
include/minigame/   public game interfaces
src/                implementation and terminal UI
tests/              deterministic core tests
docs/               gameplay and architecture notes
.github/workflows/  CI and tagged-build delivery
~~~

## Engineering highlights

- C++17 with explicit library/executable separation.
- Dependency-injected randomness so core mechanics can be tested deterministically.
- Value/RAII ownership instead of manually managed RNG pointers.
- Cross-platform line-based terminal input instead of platform-specific non-blocking terminal state.
- Automated warning-as-error builds on Linux, macOS, and Windows.
- AddressSanitizer + UndefinedBehaviorSanitizer verification on Linux.
- Tagged build workflow that produces installable artifacts.
- Regression coverage for healing, inventory accounting, combat damage, merchant purchases, progression, boss gating, and RNG bounds.

See docs/ARCHITECTURE.md for design details and docs/GAMEPLAY.md for the player-facing rules.

## History

The original July 2025 version established the exploration and battle concepts. The modernization documented in CHANGELOG.md turns that prototype into a cleaner portfolio project while preserving its terminal-game identity.
