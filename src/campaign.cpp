#include "minigame/campaign.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "minigame/random.hpp"

namespace minigame {

Campaign::Campaign(std::string playerName) : player_(std::move(playerName)) {}

Player& Campaign::player() noexcept { return player_; }
const Player& Campaign::player() const noexcept { return player_; }
int Campaign::explorations() const noexcept { return explorations_; }
int Campaign::bossesDefeated() const noexcept { return bossesDefeated_; }
int Campaign::nextBossAtWin() const noexcept { return nextBossAtWin_; }

ExploreOutcome Campaign::explore(Random& random) {
    ++explorations_;

    if (player_.wins() >= nextBossAtWin_) {
        return {ExploreType::Boss,
                "The ground shakes. A boss blocks the road ahead.",
                makeBoss(player_.level()), std::nullopt, 0};
    }

    const int roll = random.between(1, 100);
    if (roll <= 55) {
        auto enemy = makeRandomEnemy(random, player_.level());
        return {ExploreType::Battle,
                "A hostile " + enemy.name() + " steps out of the ruins.",
                std::move(enemy), std::nullopt, 0};
    }

    if (roll <= 82) {
        const Item item = random.chance(65) ? Item::Potion : Item::Bomb;
        const int gold = random.between(4, 14);
        player_.addItem(item);
        player_.addGold(gold);
        return {ExploreType::Loot,
                "You discover an abandoned cache.", std::nullopt, item, gold};
    }

    return {ExploreType::Quiet, "The road is quiet. You make progress without trouble.",
            std::nullopt, std::nullopt, 0};
}

void Campaign::resolveVictory(const Enemy& enemy) {
    player_.recordWin();
    player_.grantExperience(enemy.experienceReward());
    player_.addGold(enemy.goldReward());

    if (enemy.isBoss()) {
        ++bossesDefeated_;
        nextBossAtWin_ += 3;
        player_.addItem(Item::Potion);
        player_.addItem(Item::Bomb);
    }
}

CampaignSnapshot Campaign::snapshot() const {
    return {player_.snapshot(), explorations_, bossesDefeated_, nextBossAtWin_};
}

void Campaign::restore(const CampaignSnapshot& snapshot) {
    if (snapshot.explorations < 0 || snapshot.bossesDefeated < 0 || snapshot.nextBossAtWin < 3) {
        throw std::invalid_argument("invalid campaign snapshot");
    }
    player_.restore(snapshot.player);
    explorations_ = snapshot.explorations;
    bossesDefeated_ = snapshot.bossesDefeated;
    nextBossAtWin_ = snapshot.nextBossAtWin;
}

}  // namespace minigame
