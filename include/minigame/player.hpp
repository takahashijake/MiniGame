#pragma once

#include <string>

#include "minigame/types.hpp"

namespace minigame {

class Random;

struct PlayerSnapshot {
    std::string name;
    int health{100};
    int maxHealth{100};
    int level{1};
    int experience{0};
    int gold{0};
    int wins{0};
    int potions{0};
    int bombs{0};
};

class Player {
public:
    explicit Player(std::string name = "Adventurer");

    const std::string& name() const noexcept;
    int health() const noexcept;
    int maxHealth() const noexcept;
    int level() const noexcept;
    int experience() const noexcept;
    int gold() const noexcept;
    int wins() const noexcept;
    bool alive() const noexcept;

    int attackDamage(Random& random) const;
    int takeDamage(int amount);
    void raiseGuard() noexcept;
    bool healWithPotion();

    void addItem(Item item, int quantity = 1);
    bool consumeItem(Item item);
    int itemCount(Item item) const noexcept;

    void addGold(int amount) noexcept;
    void grantExperience(int amount);
    void recordWin() noexcept;

    PlayerSnapshot snapshot() const;
    void restore(const PlayerSnapshot& snapshot);

private:
    static int experienceNeededForLevel(int level) noexcept;

    std::string name_;
    int health_{100};
    int maxHealth_{100};
    int level_{1};
    int experience_{0};
    int gold_{0};
    int wins_{0};
    int potions_{0};
    int bombs_{0};
    bool guarding_{false};
};

}  // namespace minigame
