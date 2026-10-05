#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "minigame/campaign.hpp"
#include "minigame/combat.hpp"
#include "minigame/enemy.hpp"
#include "minigame/player.hpp"
#include "minigame/random.hpp"
#include "minigame/save_game.hpp"

namespace {
int failures = 0;

#define CHECK(condition)                                                                         \
    do {                                                                                         \
        if (!(condition)) {                                                                      \
            std::cerr << __FILE__ << ':' << __LINE__ << " CHECK failed: " #condition << '\n'; \
            ++failures;                                                                          \
        }                                                                                        \
    } while (false)

void testPlayerHealthAndGuard() {
    minigame::Player player("Tester");
    CHECK(player.takeDamage(20) == 20);
    CHECK(player.health() == 80);
    player.raiseGuard();
    CHECK(player.takeDamage(21) == 11);
    CHECK(player.health() == 69);
    CHECK(player.takeDamage(500) == 69);
    CHECK(player.health() == 0);
}

void testPotionActuallyHeals() {
    minigame::Player player("Tester");
    player.addItem(minigame::Item::Potion);
    player.takeDamage(50);
    CHECK(player.healWithPotion());
    CHECK(player.health() == 85);
    CHECK(player.itemCount(minigame::Item::Potion) == 0);
    CHECK(!player.healWithPotion());
}

void testProgressionLevelsUp() {
    minigame::Player player("Tester");
    player.takeDamage(30);
    player.grantExperience(75);
    CHECK(player.level() == 2);
    CHECK(player.maxHealth() == 115);
    CHECK(player.health() == 115);
    CHECK(player.experience() == 0);
}

void testEnemyScalingAndBoss() {
    const auto levelOne = minigame::makeEnemy(minigame::EnemyKind::Knight, 1);
    const auto levelFour = minigame::makeEnemy(minigame::EnemyKind::Knight, 4);
    CHECK(levelFour.maxHealth() > levelOne.maxHealth());
    const auto boss = minigame::makeBoss(3);
    CHECK(boss.isBoss());
    CHECK(boss.kind() == minigame::EnemyKind::Dragon);
}

void testCombatBombVictory() {
    minigame::Player player("Tester");
    player.addItem(minigame::Item::Bomb);
    minigame::Random random(7);
    minigame::Enemy fragile(minigame::EnemyKind::Knight, "Training Dummy", 10, 1, 1, 5, 5);
    minigame::CombatEngine combat(player, fragile, random);
    const auto result = combat.perform(minigame::PlayerAction::Bomb);
    CHECK(result.state == minigame::BattleState::Victory);
    CHECK(player.itemCount(minigame::Item::Bomb) == 0);
}

void testCampaignBossMilestoneAndRewards() {
    minigame::Campaign campaign("Tester");
    minigame::Enemy enemy(minigame::EnemyKind::Knight, "Dummy", 1, 1, 1, 1, 2);
    campaign.resolveVictory(enemy);
    campaign.resolveVictory(enemy);
    campaign.resolveVictory(enemy);
    CHECK(campaign.player().wins() == 3);
    CHECK(campaign.nextBossAtWin() == 3);

    minigame::Random random(5);
    const auto outcome = campaign.explore(random);
    CHECK(outcome.type == minigame::ExploreType::Boss);
    CHECK(outcome.enemy.has_value());
    CHECK(outcome.enemy->isBoss());

    campaign.resolveVictory(*outcome.enemy);
    CHECK(campaign.bossesDefeated() == 1);
    CHECK(campaign.nextBossAtWin() == 6);
    CHECK(campaign.player().itemCount(minigame::Item::Potion) == 1);
    CHECK(campaign.player().itemCount(minigame::Item::Bomb) == 1);
}

void testSaveRoundTripAndValidation() {
    minigame::Campaign original("Test Hero");
    original.player().addItem(minigame::Item::Potion, 2);
    original.player().addItem(minigame::Item::Bomb, 1);
    original.player().addGold(42);
    original.player().takeDamage(17);

    const auto path = std::filesystem::temp_directory_path() / "minigame-test-save.txt";
    std::string error;
    CHECK(minigame::SaveGame::save(original, path, error));
    const auto loaded = minigame::SaveGame::load(path, error);
    CHECK(loaded.has_value());
    CHECK(loaded->player.name == "Test Hero");
    CHECK(loaded->player.health == 83);
    CHECK(loaded->player.gold == 42);
    CHECK(loaded->player.potions == 2);
    CHECK(loaded->player.bombs == 1);
    std::filesystem::remove(path);

    const auto badPath = std::filesystem::temp_directory_path() / "minigame-test-bad-save.txt";
    {
        std::ofstream bad(badPath);
        bad << "not a valid save\n";
    }
    CHECK(!minigame::SaveGame::load(badPath, error).has_value());
    CHECK(!error.empty());
    std::filesystem::remove(badPath);
}
}  // namespace

int main() {
    testPlayerHealthAndGuard();
    testPotionActuallyHeals();
    testProgressionLevelsUp();
    testEnemyScalingAndBoss();
    testCombatBombVictory();
    testCampaignBossMilestoneAndRewards();
    testSaveRoundTripAndValidation();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All MiniGame tests passed.\n";
    return 0;
}
