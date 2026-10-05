#ifndef MINIGAME_PLAYER_H
#define MINIGAME_PLAYER_H

#include <map>
#include <string>

#include "minigame/items.h"

namespace minigame {

class RandomSource;

class Player {
public:
    static constexpr int kMaxHealth = 100;

    explicit Player(std::string name = "Adventurer");

    const std::string& name() const noexcept;
    void setName(std::string name);

    int health() const noexcept;
    bool alive() const noexcept;

    void takeDamage(int amount);
    int heal(int amount);
    bool usePotion(RandomSource& random);
    int attackDamage(RandomSource& random) const;

    void addItem(Item item, int quantity = 1);
    bool removeItem(Item item, int quantity = 1);
    bool hasItem(Item item) const;
    int itemCount(Item item) const;
    const std::map<Item, int>& inventory() const noexcept;

private:
    std::string name_;
    int health_{kMaxHealth};
    std::map<Item, int> inventory_;
};

}  // namespace minigame

#endif
