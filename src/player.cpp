#include "minigame/player.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "minigame/random.h"

namespace minigame {

Player::Player(std::string name) : name_(std::move(name)) {
    if (name_.empty()) {
        name_ = "Adventurer";
    }
}

const std::string& Player::name() const noexcept {
    return name_;
}

void Player::setName(std::string name) {
    if (!name.empty()) {
        name_ = std::move(name);
    }
}

int Player::health() const noexcept {
    return health_;
}

bool Player::alive() const noexcept {
    return health_ > 0;
}

int Player::defense() const noexcept {
    return hasItem(Item::Shield) ? kShieldReduction : 0;
}

void Player::takeDamage(int amount) {
    receiveDamage(amount);
}

int Player::receiveDamage(int amount) {
    const int mitigated = std::max(0, std::max(0, amount) - defense());
    const int previous = health_;
    health_ = std::max(0, health_ - mitigated);
    return previous - health_;
}

int Player::heal(int amount) {
    const int previous = health_;
    health_ = std::min(kMaxHealth, health_ + std::min(kMaxHealth - health_, std::max(0, amount)));
    return health_ - previous;
}

bool Player::usePotion(RandomSource& random) {
    if (!removeItem(Item::Potion)) {
        return false;
    }

    heal(random.between(25, 45));
    return true;
}

int Player::attackDamage(RandomSource& random) const {
    const int weaponBonus = hasItem(Item::Sword) ? 8 : 0;
    return random.between(14, 24) + weaponBonus;
}

void Player::addItem(Item item, int quantity) {
    if (quantity > 0) {
        int& count = inventory_[item];
        count += std::min(quantity, std::numeric_limits<int>::max() - count);
    }
}

bool Player::removeItem(Item item, int quantity) {
    if (quantity <= 0) {
        return false;
    }

    const auto found = inventory_.find(item);
    if (found == inventory_.end() || found->second < quantity) {
        return false;
    }

    found->second -= quantity;
    if (found->second == 0) {
        inventory_.erase(found);
    }
    return true;
}

bool Player::hasItem(Item item) const {
    return itemCount(item) > 0;
}

int Player::itemCount(Item item) const {
    const auto found = inventory_.find(item);
    return found == inventory_.end() ? 0 : found->second;
}

const std::map<Item, int>& Player::inventory() const noexcept {
    return inventory_;
}

}  // namespace minigame
