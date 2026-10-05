# Changelog

All notable changes to MiniGame are documented here.

## [1.0.0] - 2026-10-05

### Added

- Adventure progression loop with battle victories and a final Dragon encounter.
- Gold rewards and a travelling merchant.
- Functional Sword, Key, Gold, and Potion item roles.
- Boss gate that requires three victories and a Key.
- Deterministic core test suite.
- CMake build with warning-as-error and sanitizer options.
- Cross-platform CI for Linux, macOS, and Windows.
- Tagged-build artifact delivery workflow.
- Architecture, gameplay, and contribution documentation.

### Changed

- Reorganized production code into include/minigame and src.
- Replaced platform-specific non-blocking input with portable line-based commands.
- Reworked combat around explicit Player, Character, BattleSequence, Progression, and RandomSource responsibilities.
- Replaced raw RNG ownership with RAII/value ownership.
- Improved terminal presentation with a persistent HUD, health bar, clearer commands, merchant UI, and victory condition.
- Modernized the convenience Makefile to drive CMake.

### Fixed

- Potions now restore HP instead of subtracting HP.
- Health values are clamped and cannot become negative or exceed maximum HP.
- Inventory removal cannot create negative quantities.
- Duplicate permanent-item purchases no longer spend Gold.
- Removed the committed battle.o build artifact.

## [0.1.0] - 2025-07-24

Legacy prototype baseline: walking events, Knight/Mage encounters, randomized enemy movement, inventory scaffolding, and a separated battle sequence.
