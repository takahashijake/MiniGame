# Changelog

All notable changes to MiniGame are documented here.

## [2.0.0] - 2026-10-05

### Added
- Campaign progression with XP, levels, gold, boss milestones, and enemy scaling.
- Versioned save/load system with validation.
- Bomb and defend combat actions.
- New Berserker enemy and recurring Ember Dragon boss encounters.
- Terminal dashboard, health bars, inventory view, battle view, color controls, and accessibility-friendly no-clear mode.
- Cross-platform CMake build and install target.
- Unit/integration test executable wired into CTest.
- GitHub Actions CI for Linux, macOS, and Windows.
- Sanitizer, warning-as-error, and clang-format quality gates.
- Tagged-release workflow that builds and publishes packaged binaries.
- Architecture and contribution documentation.

### Changed
- Migrated the codebase from C++14 to C++17.
- Split game logic from terminal I/O so core systems can be tested without interactive input.
- Replaced raw owning pointers and manual allocation with value semantics and RAII.
- Replaced platform-specific nonblocking input with a portable line-based command interface.
- Reorganized the repository into `include/`, `src/`, `tests/`, and `docs/`.

### Fixed
- Player potion healing previously reduced health instead of restoring it.
- Damage now clamps at zero health and guard state is consumed predictably.
- Inventory consumption no longer creates negative counts.
- Save data is validated before being accepted.
- Removed generated object files from source control.

## [1.0.0]

- Original turn-based terminal prototype with Knight and Mage encounters, random loot, inventory, and combat.
