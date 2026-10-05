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

void testSwordAddsDamage() {
    minigame::Player player;
    SequenceRandom random({14});

    const int unarmed = player.attackDamage(random);
    player.addItem(minigame::Item::Sword);
    const int armed = player.attackDamage(random);

    check(unarmed == 14, "base attack should use the random damage roll");
    check(armed == 22, "sword should add eight damage");
}

void testInventoryCannotGoNegative() {
    minigame::Player player;
    check(!player.removeItem(minigame::Item::Gold), "cannot remove a missing item");
    player.addItem(minigame::Item::Gold, 2);
    check(!player.removeItem(minigame::Item::Gold, 3), "cannot overspend inventory");
    check(player.itemCount(minigame::Item::Gold) == 2, "failed removal should not mutate inventory");
}

void testProgressionAndBossGate() {
    minigame::Player player;
    minigame::Progression progression;
    SequenceRandom random({2, 100, 2, 100, 2, 100});

    progression.recordVictory(player, random);
    progression.recordVictory(player, random);
    progression.recordVictory(player, random);

    check(progression.victories() == 3, "victories should accumulate");
    check(player.itemCount(minigame::Item::Gold) == 6, "victories should award gold");
    check(!progression.bossAvailable(player), "boss should still require a key");

    player.addItem(minigame::Item::Key);
    check(progression.bossAvailable(player), "boss should unlock after victories and key");
    check(progression.openBossGate(player), "key should open the gate");
    check(!player.hasItem(minigame::Item::Key), "opening the gate should consume the key");
    check(progression.bossAvailable(player), "opened gate should remain available");
    check(!progression.purchase(player, minigame::Item::Key),
          "an opened gate should reject another key purchase");
}

void testMerchantPurchases() {
    minigame::Player player;
    minigame::Progression progression;
    player.addItem(minigame::Item::Gold, 10);

    check(progression.purchase(player, minigame::Item::Sword), "sword purchase should succeed");
    check(player.hasItem(minigame::Item::Sword), "purchased sword should enter inventory");
    check(player.itemCount(minigame::Item::Gold) == 2, "purchase should deduct gold");
    check(!progression.purchase(player, minigame::Item::Sword), "duplicate sword should be rejected");
    check(player.itemCount(minigame::Item::Gold) == 2, "rejected purchase should not spend gold");
}

void testCharacterHealthRules() {
    minigame::Knight knight;
    knight.takeDamage(500);
    check(!knight.alive(), "enemy should die at zero health");
    check(knight.health() == 0, "enemy health should not become negative");
    knight.heal(500);
    check(knight.health() == knight.maxHealth(), "enemy healing should cap at max health");
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

    const std::string json = engine.stateJson();
    check(json.find("\"playerName\":\"WebTester\"") != std::string::npos,
          "web state should serialize the player name");
    check(json.find("\"victories\":1") != std::string::npos,
          "web state should serialize progression");
}

}  // namespace

int main() {
    testPotionHealsInsteadOfDamaging();
    testHealthIsClamped();
    testSwordAddsDamage();
    testInventoryCannotGoNegative();
    testProgressionAndBossGate();
    testMerchantPurchases();
    testCharacterHealthRules();
    testRandomBounds();
    testGameEngineEncounterFlow();

    if (failures == 0) {
        std::cout << "All MiniGame core tests passed.\n";
        return 0;
    }

    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
