#pragma once

#include <string>

#include "minigame/types.hpp"

namespace minigame {

class Random;

class Enemy {
public:
    Enemy(EnemyKind kind, std::string name, int health, int minDamage, int maxDamage,
          int experienceReward, int goldReward, bool boss = false);

    EnemyKind kind() const noexcept;
    const std::string& name() const noexcept;
    int health() const noexcept;
    int maxHealth() const noexcept;
    int experienceReward() const noexcept;
    int goldReward() const noexcept;
    bool isBoss() const noexcept;
    bool defeated() const noexcept;

    int takeDamage(int amount);
    int attackDamage(Random& random) const;
    int heal(Random& random);

private:
    EnemyKind kind_;
    std::string name_;
    int health_;
    int maxHealth_;
    int minDamage_;
    int maxDamage_;
    int experienceReward_;
    int goldReward_;
    bool boss_;
};

Enemy makeEnemy(EnemyKind kind, int playerLevel);
Enemy makeRandomEnemy(Random& random, int playerLevel);
Enemy makeBoss(int playerLevel);

}  // namespace minigame
