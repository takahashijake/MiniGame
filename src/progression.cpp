#include "minigame/progression.h"

#include "minigame/player.h"
#include "minigame/random.h"

namespace minigame {

int Progression::victories() const noexcept {
    return victories_;
}

bool Progression::gateOpened() const noexcept {
    return gateOpened_;
}

bool Progression::bossDefeated() const noexcept {
    return bossDefeated_;
}

BattleReward Progression::recordVictory(Player& player, RandomSource& random) {
    ++victories_;

    BattleReward reward;
    reward.gold = random.between(2, 5);
    player.addItem(Item::Gold, reward.gold);

    reward.potionDropped = random.chance(25);
    if (reward.potionDropped) {
        player.addItem(Item::Potion);
    }

    return reward;
}

int Progression::price(Item item) const noexcept {
    switch (item) {
        case Item::Potion:
            return 3;
        case Item::Sword:
            return 8;
        case Item::Key:
            return 6;
        case Item::Gold:
            return -1;
    }
    return -1;
}

bool Progression::purchase(Player& player, Item item) const {
    const int cost = price(item);
    if (cost <= 0 || player.itemCount(Item::Gold) < cost) {
        return false;
    }

    if (item == Item::Sword && player.hasItem(Item::Sword)) {
        return false;
    }

    if (item == Item::Key && (player.hasItem(Item::Key) || gateOpened_)) {
        return false;
    }

    if (!player.removeItem(Item::Gold, cost)) {
        return false;
    }

    player.addItem(item);
    return true;
}

bool Progression::bossAvailable(const Player& player) const {
    if (victories_ < kVictoriesForBoss) {
        return false;
    }
    return gateOpened_ || player.hasItem(Item::Key);
}

bool Progression::openBossGate(Player& player) {
    if (victories_ < kVictoriesForBoss) {
        return false;
    }

    if (gateOpened_) {
        return true;
    }

    if (!player.removeItem(Item::Key)) {
        return false;
    }

    gateOpened_ = true;
    return true;
}

void Progression::recordBossDefeat() noexcept {
    bossDefeated_ = true;
}

}  // namespace minigame
