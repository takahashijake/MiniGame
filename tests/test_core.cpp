#include <algorithm>
#include <iostream>
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
    auto random = std::make_unique<SequenceRandom>(
        std::vector<int>{1, 1, 14, 1, 20, 14, 1, 20, 14, 1, 20, 14, 1, 20, 14, 1, 20});
    minigame::GameEngine engine("Doomed", std::move(random));

    engine.perform("walk");
    while (engine.snapshot().phase == "battle") {
        engine.perform("attack");
    }

    if (engine.snapshot().phase == "defeat") {
        minigame::GameEngine restored("Placeholder", 5u);
        check(restored.loadState(engine.saveState()), "defeat state should be loadable");
        check(restored.snapshot().phase == "defeat", "restored defeat should remain terminal");
        check(restored.snapshot().enemyName.empty(), "defeat should not retain an active enemy");
    }
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

    if (failures == 0) {
        std::cout << "All MiniGame core tests passed.\n";
        return 0;
    }

    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
