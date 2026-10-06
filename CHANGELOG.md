# Changelog

## [Unreleased] — v1.2 review fixes (2026-10-06)

- Make seeded RNG mapping portable between native and WebAssembly standard libraries.
- Validate saves atomically, including flags, phase/gate invariants and complete RNG data.
- Reject invalid native seeds; quit cleanly on EOF; allow load and quit during combat.
- Keep the browser playable when localStorage is denied, and clear corrected seed validation.
- Escape every JSON control character; prevent restored reward counters from overflowing.
- Split persistence validation and browser rendering into focused helpers.
- Preserve unrelated service-worker caches and validate the entire shell before caching.
- Add deterministic combat, CLI, browser/WASM, RNG parity and service-worker regressions to CI.
- Document persistence limits, offline behavior, turn costs and QA commands.

## [1.2.0] - 2026-10-05

### Added

- Browser autosave and Continue Run backed by the C++ engine save format.
- Native .minigame-save save/load, including combat saves.
- Optional deterministic 32-bit run seeds.
- RNG-state serialization for exact continuation after loading.
- Shield equipment with permanent 5-point damage mitigation.
- Rogue glass-cannon enemy.
- Golem tank enemy.
- Healing shrine exploration event.
- Dragon phase two: 180 HP, no escape, +8 enraged damage at or below half health.
- HUD state for seed, defense, Shield, and Dragon enrage.
- Regression tests for seeded reproducibility, save/load RNG continuity, Shield behavior, and new enemies.

### Changed

- Exploration now selects from four normal enemy archetypes.
- Loot can award a Shield.
- Merchant sells Shield for 10 Gold.
- Project version is 1.2.0.

## [1.1.0] - 2026-10-05

- Added browser product compiled from the C++ core to WebAssembly.
- Added responsive frontend and shared GameEngine.
- Added WebAssembly CI and browser release artifacts.

## [1.0.0] - 2026-10-05

- Modernized repository architecture, tests, CI/CD, docs, and terminal UX.
- Added adventure progression, merchant, functional inventory items, and Dragon win condition.
- Fixed Potion healing, health bounds, inventory accounting, and raw RNG ownership.

## [0.1.0] - 2025-07-24

Legacy terminal prototype baseline.
