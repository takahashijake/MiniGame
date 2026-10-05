#include "minigame/enemy.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "minigame/random.hpp"

namespace minigame {

Enemy::Enemy(EnemyKind kind, std::string name, int health, int minDamage, int maxDamage,
             int experienceReward, int goldReward, bool boss)
    : kind_(kind),
      name_(std::move(name)),
      health_(health),
      maxHealth_(health),
      minDamage_(minDamage),
      maxDamage_(maxDamage),
      experienceReward_(experienceReward),
      goldReward_(goldReward),
      boss_(boss) {
    if (health <= 0 || minDamage < 0 || minDamage > maxDamage || experienceReward < 0 ||
        goldReward < 0) {
        throw std::invalid_argument("invalid enemy stats");
    }
}

EnemyKind Enemy::kind() const noexcept { return kind_; }
const std::string& Enemy::name() const noexcept { return name_; }
int Enemy::health() const noexcept { return health_; }
int Enemy::maxHealth() const noexcept { return maxHealth_; }
int Enemy::experienceReward() const noexcept { return experienceReward_; }
int Enemy::goldReward() const noexcept { return goldReward_; }
bool Enemy::isBoss() const noexcept { return boss_; }
bool Enemy::defeated() const noexcept { return health_ <= 0; }

int Enemy::takeDamage(int amount) {
    if (amount <= 0 || defeated()) {
        return 0;
    }
    const int applied = std::min(amount, health_);
    health_ -= applied;
    return applied;
}

int Enemy::attackDamage(Random& random) const { return random.between(minDamage_, maxDamage_); }

int Enemy::heal(Random& random) {
    if (defeated() || health_ >= maxHealth_) {
        return 0;
    }
    const int before = health_;
    health_ = std::min(maxHealth_, health_ + random.between(10, 22));
    return health_ - before;
}

Enemy makeEnemy(EnemyKind kind, int playerLevel) {
    const int scale = std::max(0, playerLevel - 1);
    switch (kind) {
        case EnemyKind::Knight:
            return {kind, "Iron Knight", 62 + scale * 11, 8 + scale * 2, 14 + scale * 2,
                    32 + scale * 4, 16 + scale * 3};
        case EnemyKind::Mage:
            return {kind, "Ash Mage", 50 + scale * 9, 10 + scale * 2, 17 + scale * 2,
                    38 + scale * 4, 20 + scale * 3};
        case EnemyKind::Berserker:
            return {kind, "Wild Berserker", 78 + scale * 13, 11 + scale * 2,
                    20 + scale * 3, 45 + scale * 5, 24 + scale * 3};
        case EnemyKind::Dragon:
            return makeBoss(playerLevel);
    }
    return {EnemyKind::Knight, "Iron Knight", 62, 8, 14, 32, 16};
}

Enemy makeRandomEnemy(Random& random, int playerLevel) {
    const int roll = random.between(0, 2);
    return makeEnemy(static_cast<EnemyKind>(roll), playerLevel);
}

Enemy makeBoss(int playerLevel) {
    const int scale = std::max(0, playerLevel - 1);
    return {EnemyKind::Dragon, "Ember Dragon", 150 + scale * 24, 14 + scale * 2,
            24 + scale * 3, 130 + scale * 15, 80 + scale * 10, true};
}

}  // namespace minigame
