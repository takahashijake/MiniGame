# Architecture

MiniGame 2.x uses a small layered design so gameplay rules are independent from the terminal front end.

## Layers

### Core domain

`Player`, `Enemy`, `CombatEngine`, and `Campaign` contain the game rules. These types do not read from stdin, clear the screen, sleep, or depend on platform terminal APIs. `Random` is explicitly passed into systems that need randomness so tests can use a fixed seed.

### Persistence

`SaveGame` converts a `CampaignSnapshot` to and from a versioned text format. Loading performs syntax checks and domain validation before state is restored. The version header makes future migrations explicit instead of silently accepting incompatible saves.

### Presentation

`TerminalUI` owns terminal rendering, prompts, ANSI color, status bars, and accessibility toggles. `main.cpp` coordinates application flow but delegates rules to the core types.

## Major gameplay flow

1. The player starts or loads a campaign.
2. `Campaign::explore` selects a regular encounter, loot, a quiet event, or a pending boss.
3. `CombatEngine` resolves one player action and one enemy response per turn.
4. `Campaign::resolveVictory` grants XP and gold and advances boss progression.
5. Every third regular victory makes the next exploration a boss encounter.
6. Boss victories award bonus supplies and move the next boss milestone forward.

## Design decisions

- **Value semantics first:** enemies and snapshots are values; ownership is explicit without manual `new`/`delete`.
- **Dependency-free tests:** the test runner uses simple assertions to keep the project lightweight.
- **Portable input:** line-based commands work consistently in normal terminals and CI environments.
- **Versioned persistence:** save incompatibility fails with a useful message rather than undefined behavior.
- **No UI in the engine:** future front ends can reuse the game core without rewriting gameplay logic.

## Extension points

Good next additions include shops, equipment, status effects, encounter tables loaded from data, additional save-version migrations, and a graphical front end that links against `minigame_core`.
