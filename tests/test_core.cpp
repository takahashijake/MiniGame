#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "minigame/character.h"
#include "minigame/game_engine.h"
#include "minigame/items.h"
#include "minigame/player.h"
#include "minigame/progression.h"
#include "minigame/random.h"

namespace {

class SequenceRandom final : public minigame::RandomSource {
public:
    explicit SequenceRandom(std::vector<int> values) : values_(std::move(values)) {}

    int between(int minimum, int maximum) override {
        if (values_.empty()) {
            return minimum;
        }

        const int value = values_[index_ % values_.size()];
        ++index_;
        return std::clamp(value, minimum, maximum);
    }

private:
    std::vector<int> values_;
    std::size_t index_{0};
};

int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << "\n";
    }
}

void testPotionHealsInsteadOfDamaging() {
    minigame::Player player("Tester");
    player.takeDamage(60);
    player.addItem(minigame::Item::Potion);

    SequenceRandom random({30});
    check(player.usePotion(random), "potion should be usable when present");
    check(player.health() == 70, "potion should restore health");
    check(player.itemCount(minigame::Item::Potion) == 0, "potion should be consumed");
}

void testHealthIsClamped() {
    minigame::Player player;
    player.takeDamage(1000);
    check(player.health() == 0, "damage should not make health negative");
    player.heal(1000);
    check(player.health() == minigame::Player::kMaxHealth, "healing should cap at max health");
}

void testEquipmentEffects() {
    minigame::Player player;
    SequenceRandom random({14});

    const int unarmed = player.attackDamage(random);
    player.addItem(minigame::Item::Sword);
    const int armed = player.attackDamage(random);

    check(unarmed == 14, "base attack should use the random damage roll");
    check(armed == 22, "sword should add eight damage");

    player.addItem(minigame::Item::Shield);
    const int applied = player.receiveDamage(20);
    check(player.defense() == 5, "shield should expose five defense");
    check(applied == 15, "shield should reduce incoming damage by five");
    check(player.health() == 85, "mitigated damage should update player health");
}

void testInventoryCannotGoNegative() {
    minigame::Player player;
    check(!player.removeItem(minigame::Item::Gold), "cannot remove a missing item");
    player.addItem(minigame::Item::Gold, 2);
    check(!player.removeItem(minigame::Item::Gold, 3), "cannot overspend inventory");
    check(player.itemCount(minigame::Item::Gold) == 2, "failed removal should not mutate inventory");
}

void testProgressionAndMerchant() {
    minigame::Player player;
    minigame::Progression progression;
    SequenceRandom random({2, 100, 2, 100, 2, 100});

    progression.recordVictory(player, random);
    progression.recordVictory(player, random);
    progression.recordVictory(player, random);

    check(progression.victories() == 3, "victories should accumulate");
    check(player.itemCount(minigame::Item::Gold) == 6, "victories should award gold");
    check(!progression.bossAvailable(player), "boss should still require a key");

    player.addItem(minigame::Item::Gold, 20);
    check(progression.purchase(player, minigame::Item::Shield), "shield purchase should succeed");
    check(player.hasItem(minigame::Item::Shield), "purchased shield should enter inventory");
    check(!progression.purchase(player, minigame::Item::Shield),
          "duplicate shield purchase should be rejected");

    player.addItem(minigame::Item::Key);
    check(progression.bossAvailable(player), "boss should unlock after victories and key");
    check(progression.openBossGate(player), "key should open the gate");
    check(!player.hasItem(minigame::Item::Key), "opening the gate should consume the key");
    check(progression.bossAvailable(player), "opened gate should remain available");
    check(!progression.purchase(player, minigame::Item::Key),
          "an opened gate should reject another key purchase");
}

void testEnemyArchetypes() {
    SequenceRandom random({20});

    minigame::Rogue rogue;
    minigame::Golem golem;
    minigame::Dragon dragon;

    check(rogue.maxHealth() == 65, "rogue should be the low-health glass cannon");
    check(golem.maxHealth() == 125, "golem should be the tank archetype");
    check(dragon.maxHealth() == 180, "dragon should have expanded boss health");
    check(rogue.attackDamage(random) >= 16, "rogue should deal high base damage");
    check(golem.attackDamage(random) <= 19, "golem should trade damage for durability");
}

void testRandomBounds() {
    minigame::RandomGenerator random(42);
    for (int i = 0; i < 200; ++i) {
        const int value = random.between(3, 7);
        check(value >= 3 && value <= 7, "random values should remain inside requested bounds");
    }
}

void testGameEngineEncounterFlow() {
    auto random = std::make_unique<SequenceRandom>(
        std::vector<int>{1, 1, 24, 1, 10, 24, 1, 10, 24, 1, 10, 24, 5, 100});
    minigame::GameEngine engine("WebTester", std::move(random));

    engine.perform("walk");
    auto state = engine.snapshot();
    check(state.phase == "battle", "walking should be able to start a battle");
    check(state.enemyName == "Knight", "deterministic encounter should spawn a Knight");

    engine.perform("attack");
    engine.perform("attack");
    engine.perform("attack");
    engine.perform("attack");

    state = engine.snapshot();
    check(state.phase == "exploring", "winning should return to exploration");
    check(state.victories == 1, "engine should record a normal victory");
    check(state.gold == 5, "engine should award deterministic battle gold");
    check(state.health == 70, "enemy turns should damage the player between attacks");
    check(state.enemyName.empty(), "resolved battles should clear the active enemy");
}

void testSeededRunsAreReproducible() {
    minigame::GameEngine first("SeedTester", 123456u);
    minigame::GameEngine second("SeedTester", 123456u);

    for (int turn = 0; turn < 18; ++turn) {
        check(first.stateJson() == second.stateJson(),
              "identical seeds and commands should produce identical state");

        const auto state = first.snapshot();
        if (state.phase == "victory" || state.phase == "defeat") {
            break;
        }

        const std::string command = state.phase == "battle" ? "attack" : "walk";
        first.perform(command);
        second.perform(command);
    }

    check(first.snapshot().seed == 123456u, "snapshot should expose the run seed");
}

void testDefeatStateCanBePersisted() {
    minigame::GameEngine engine("Doomed", 42u);
    for (int i = 0; i < 300 && engine.snapshot().phase != "defeat"; ++i) {
        engine.perform(engine.snapshot().phase == "battle" ? "attack" : "walk");
    }
    check(engine.snapshot().phase == "defeat", "fixture must actually reach defeat");
    minigame::GameEngine restored("Placeholder", 5u);
    check(restored.loadState(engine.saveState()), "defeat state should be loadable");
    check(restored.stateJson() == engine.stateJson(), "defeat snapshot must round trip");
}

void testSaveLoadPreservesRngContinuity() {
    minigame::GameEngine original("Saver", 98765u);

    for (int turn = 0; turn < 8; ++turn) {
        const auto state = original.snapshot();
        original.perform(state.phase == "battle" ? "attack" : "walk");
        if (original.snapshot().phase == "defeat") {
            break;
        }
    }

    const std::string save = original.saveState();
    minigame::GameEngine restored("Placeholder", 1u);

    check(restored.loadState(save), "valid save should load");
    check(restored.stateJson() == original.stateJson(),
          "loaded state should exactly match the saved engine snapshot");

    const auto current = original.snapshot();
    if (current.phase != "victory" && current.phase != "defeat") {
        const std::string nextCommand = current.phase == "battle" ? "attack" : "walk";
        original.perform(nextCommand);
        restored.perform(nextCommand);
        check(restored.stateJson() == original.stateJson(),
              "loaded RNG state should preserve the next deterministic transition");
    }

    check(!restored.loadState("not-a-minigame-save"), "invalid save data should be rejected");
}

// Handwritten MG2 fixtures exercise the public restore boundary, using a real RNG state.
std::string fixture(const std::string& fields) {
    minigame::RandomGenerator random(42u);
    std::ostringstream save;
    save << "MG2 42 \"Tester\" " << fields << " \"fixture\" "
         << std::quoted(random.serializeState());
    return save.str();
}

void testInvalidSavesAreAtomic() {
    minigame::GameEngine engine("Untouched", 19u);
    engine.perform("walk");
    const auto before = engine.saveState();
    const std::vector<std::string> invalid = {
        fixture("100 0 0 2 0 0 0 0 0 0 0 \"\" 0"), // nonboolean sword
        fixture("0 0 0 0 0 0 0 0 0 0 0 \"\" 0"), // dead explorer
        fixture("100 0 0 0 0 0 0 0 0 2 0 \"\" 0"), // living defeat
        fixture("100 0 0 0 0 0 0 0 0 1 0 \"Knight\" 0"), // dead combat enemy
        fixture("100 0 0 0 0 0 0 0 0 1 0 \"Dragon\" 180"), // bypass boss gate
        fixture("100 0 0 0 0 0 0 0 0 0 1 \"\" 0"), // boss outside battle
        fixture("100 0 0 0 0 0 0 0 0 3 0 \"\" 0"), // unearned victory
        fixture("100 0 0 0 0 0 0 1 0 0 0 \"\" 0"), // unopened progression
        "MG2 -1" + before.substr(before.find(' ', 4)),
        "MG2 4294967296" + before.substr(before.find(' ', 4)),
        before + " trailing garbage",
        before.substr(0, before.size() / 2)
    };
    for (const auto& save : invalid) {
        check(!engine.loadState(save), "malformed or inconsistent save must be rejected");
        check(engine.saveState() == before, "failed load must preserve all state including RNG");
    }
    minigame::Player rich;
    rich.addItem(minigame::Item::Gold, std::numeric_limits<int>::max());
    rich.addItem(minigame::Item::Gold, 5);
    check(rich.itemCount(minigame::Item::Gold) == std::numeric_limits<int>::max(),
          "large restored counters must not overflow on rewards");
    minigame::Progression progression;
    progression.restore(std::numeric_limits<int>::max(), false, false);
    SequenceRandom reward({5, 100});
    progression.recordVictory(rich, reward);
    check(progression.victories() == std::numeric_limits<int>::max(),
          "restored victory counter must not overflow");
    minigame::RandomGenerator random(42u);
    const auto rngBefore = random.serializeState();
    check(!random.restoreState(rngBefore + " junk"), "RNG parser must consume the entire payload");
    check(random.serializeState() == rngBefore, "failed RNG restore must be atomic");
}

void testDragonAndVictoryPersistence() {
    auto random = std::make_unique<SequenceRandom>(std::vector<int>{14, 1, 20});
    minigame::GameEngine engine("BossTester", std::move(random));
    check(engine.loadState(fixture("100 0 2 1 1 0 3 1 0 1 1 \"Dragon\" 90")),
          "enraged Dragon save should load");
    check(engine.snapshot().enemyEnraged, "Dragon enrages at exactly half HP");
    minigame::GameEngine restored("Other", 1u);
    check(restored.loadState(engine.saveState()), "mid-boss save should load");
    check(restored.stateJson() == engine.stateJson(), "shield and enrage must survive restore");
    engine.perform("run");
    restored.perform("run");
    check(engine.snapshot().phase == "battle", "Dragon escape must remain blocked");
    check(engine.snapshot().message.find("no escape") != std::string::npos,
          "blocked escape should report its consequence");
    check(restored.stateJson() == engine.stateJson(), "resumed boss RNG must match");

    check(engine.loadState(fixture("100 0 0 1 1 0 3 1 0 1 1 \"Dragon\" 1")),
          "near-victory fixture should load");
    engine.perform("attack");
    check(engine.snapshot().phase == "victory", "lethal boss hit must complete run");
    check(restored.loadState(engine.saveState()), "victory must round trip");
    check(restored.stateJson() == engine.stateJson(), "victory snapshot must match");
    restored.perform("walk");
    check(restored.snapshot().phase == "victory", "completed run must remain terminal");
    restored.resetSeeded("Fresh", 0u);
    check(restored.snapshot().phase == "exploring" && !restored.snapshot().gateOpened &&
          !restored.snapshot().hasShield && restored.snapshot().seed == 0,
          "new seeded run must clear old equipment/progression");
}

void testCrossPlatformRandomVector() {
    minigame::RandomGenerator random(42u);
    const std::vector<int> expected{43, 68, 77, 15, 27, 36, 21, 25};
    for (const int value : expected) {
        check(random.between(1, 100) == value, "portable seeded RNG must match golden vector");
    }
    check(random.between(std::numeric_limits<int>::min(), std::numeric_limits<int>::max())
              >= std::numeric_limits<int>::min(), "full-width integer range must be supported");
    minigame::GameEngine engine(std::string("A\x01\b\f", 4), 42u);
    check(engine.stateJson().find("\\u0001") != std::string::npos,
          "JSON must escape arbitrary control characters");
}

void testScriptedCombatRules() {
    // Acquire gear, two potions and a key; win three Knights; heal; fight Dragon.
    std::vector<int> rolls{59, 60, 59, 75, 59, 40, 59, 1, 59, 1};
    for (int fight = 0; fight < 3; ++fight) {
        const std::vector<int> knight{1, 1, 24, 1, 10, 24, 1, 10, 24, 5, 100};
        rolls.insert(rolls.end(), knight.begin(), knight.end());
    }
    const std::vector<int> boss{83, 30, 24, 1, 20, 24, 1, 20, 24, 1, 20,
                              1, 20, 40, 1, 20, 24, 1, 20, 40, 1, 20,
                              24, 1, 20, 24};
    rolls.insert(rolls.end(), boss.begin(), boss.end());
    minigame::GameEngine engine("Scripted", std::make_unique<SequenceRandom>(rolls));
    for (int i = 0; i < 5; ++i) engine.perform("walk");
    for (int i = 0; i < 3; ++i) {
        engine.perform("walk");
        for (int hit = 0; hit < 3; ++hit) engine.perform("attack");
    }
    check(engine.snapshot().victories == 3, "script must unlock boss progression");
    engine.perform("walk");
    engine.perform("boss");
    check(!engine.snapshot().hasKey && engine.snapshot().gateOpened,
          "boss gate consumes the key exactly once");
    engine.perform("attack"); engine.perform("attack");
    check(!engine.snapshot().enemyEnraged && engine.snapshot().health == 70,
          "above half HP Dragon attacks for 20 minus five Shield defense");
    engine.perform("attack");
    check(engine.snapshot().enemyHealth == 84 && engine.snapshot().enemyEnraged &&
          engine.snapshot().health == 47,
          "crossing half HP immediately adds eight damage before Shield mitigation");
    engine.perform("run");
    check(engine.snapshot().health == 24 && engine.snapshot().enemyHealth == 84,
          "blocked Dragon flee costs one enemy turn and does not damage the boss");
    engine.perform("heal");
    check(engine.snapshot().health == 41 && engine.snapshot().potions == 1,
          "potion consumes one item and costs an enraged enemy turn");
    engine.perform("attack"); engine.perform("heal"); engine.perform("attack");
    check(engine.snapshot().health == 12, "scripted combat must apply precise damage");
    engine.perform("heal");
    check(engine.snapshot().health == 12, "missing potion must not consume an enemy turn");
    engine.perform("attack");
    check(engine.snapshot().phase == "victory" && engine.snapshot().enemyName.empty(),
          "lethal attack must skip retaliation and clear the boss");

    minigame::GameEngine rogue("Rogue", std::make_unique<SequenceRandom>(
        std::vector<int>{1, 60, 14, 80, 16}));
    rogue.perform("walk"); rogue.perform("attack");
    check(rogue.snapshot().enemyName == "Rogue" && rogue.snapshot().enemyHealth == 51 &&
          rogue.snapshot().health == 84, "Rogue must attack rather than heal on a healing roll");
    minigame::GameEngine golem("Golem", std::make_unique<SequenceRandom>(
        std::vector<int>{1, 82, 24, 80, 8}));
    golem.perform("walk"); golem.perform("attack");
    check(golem.snapshot().enemyName == "Golem" && golem.snapshot().enemyHealth == 109 &&
          golem.snapshot().health == 100, "Golem healing consumes its turn");
}

void testManyResumedRuns() {
    for (std::uint32_t seed = 0; seed < 32; ++seed) {
        minigame::GameEngine original("Replay", seed);
        minigame::GameEngine resumed("Other", 999u);
        for (int turn = 0; turn < 80; ++turn) {
            check(resumed.loadState(original.saveState()), "every live phase must be loadable");
            check(resumed.stateJson() == original.stateJson(), "live snapshot must match");
            const auto state = original.snapshot();
            if (state.phase == "defeat" || state.phase == "victory") break;
            const std::string command = state.phase == "battle" ?
                (turn % 5 == 0 ? "run" : (turn % 3 == 0 ? "heal" : "attack")) :
                (turn % 4 == 0 ? "buy:shield" : "walk");
            original.perform(command);
            resumed.perform(command);
            check(original.saveState() == resumed.saveState(), "resumed transition/RNG must match");
        }
    }
}

}  // namespace

int main() {
    testPotionHealsInsteadOfDamaging();
    testHealthIsClamped();
    testEquipmentEffects();
    testInventoryCannotGoNegative();
    testProgressionAndMerchant();
    testEnemyArchetypes();
    testRandomBounds();
    testGameEngineEncounterFlow();
    testSeededRunsAreReproducible();
    testDefeatStateCanBePersisted();
    testSaveLoadPreservesRngContinuity();
    testInvalidSavesAreAtomic();
    testDragonAndVictoryPersistence();
    testCrossPlatformRandomVector();
    testManyResumedRuns();
    testScriptedCombatRules();

    if (failures == 0) {
        std::cout << "All MiniGame core tests passed.\n";
        return 0;
    }

    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
