# Architecture

MiniGame is split into a small reusable game core and a terminal-facing orchestration layer.

## Components

- Player owns player health and inventory rules. It knows how much damage the player deals and how Potions are consumed.
- Character is the polymorphic enemy base class. Knight, Mage, and Dragon provide different health and attack/heal ranges.
- BattleSequence owns no game entities. It coordinates turn order using references to a Player, Character, RandomSource, input stream, and output stream.
- Progression tracks normal victories, merchant pricing, the boss gate, and final-boss completion.
- RandomSource is the test seam for randomness. RandomGenerator is the production implementation backed by std::mt19937.
- GameState owns the long-lived session state and terminal menu. It selects exploration events and connects battles to progression rewards.

## Dependency direction

The public interfaces live under include/minigame. Implementation lives under src. The executable entry point only constructs GameState. Tests link against minigame_core and can replace RandomGenerator with deterministic RandomSource implementations.

This separation deliberately removes two problems from the prototype: gameplay classes no longer own heap-allocated RNG objects, and tests no longer need a real terminal or unpredictable random values.

## State flow

1. GameState reads a main-menu command.
2. Walk selects an exploration event.
3. A combat event constructs an enemy and BattleSequence.
4. BattleSequence returns a BattleResult.
5. GameState turns normal victories into Progression rewards.
6. After three victories and a Key, Progression opens the boss gate.
7. A Dragon victory marks the run complete.

The boss gate stays open after the Key is consumed, so fleeing from the Dragon does not require purchasing another Key.
