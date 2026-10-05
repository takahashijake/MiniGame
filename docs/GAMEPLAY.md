# Gameplay Guide

## Objective

Win three normal battles, secure a Key, open the ancient gate, and defeat the Dragon.

## Exploration

Exploring can produce an enemy encounter, loot, a healing shrine restoring 15–30 HP, or a quiet road event.

Normal encounters include Knight, Mage, Rogue, and Golem.

## Enemies

- Knight: balanced and durable.
- Mage: lighter health with stronger healing.
- Rogue: 65 HP glass cannon with high damage and no healing.
- Golem: 125 HP tank with lower damage.
- Dragon: 180 HP final boss. The player cannot flee. At or below half health, the Dragon becomes Enraged and adds 8 attack damage before Shield mitigation.

## Combat

- Attack: normal player damage; Sword adds 8.
- Potion: consume one Potion and restore 25–45 HP.
- Run: 35% escape chance outside the Dragon arena.

## Equipment

- Sword: permanent +8 attack damage.
- Shield: permanent 5-point incoming-damage reduction.
- Potion: consumable healing.
- Key: opens the boss gate and is consumed on first use.

## Merchant

- Potion: 3 Gold
- Key: 6 Gold
- Sword: 8 Gold
- Shield: 10 Gold

Permanent equipment cannot be purchased twice.

## Seeds

Every run has a visible unsigned 32-bit seed. Identical seeds and commands reproduce the same random sequence.

## Persistence

The native CLI saves to .minigame-save.

The browser autosaves to localStorage after each engine action and exposes Continue saved run.

Persistence includes the live random-engine state, so resumed runs continue the same sequence exactly.
