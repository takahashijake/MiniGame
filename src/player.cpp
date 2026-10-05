#include "minigame/player.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "minigame/random.hpp"

namespace minigame {

Player::Player(std::string name) : name_(std::move(name)) {
    if (name_.empty()) {
        name_ = "Adventurer";
    }
}

const std::string& Player::name() const noexcept { return name_; }
int Player::health() const noexcept { return health_; }
int Player::maxHealth() const noexcept { return maxHealth_; }
int Player::level() const noexcept { return level_; }
int Player::experience() const noexcept { return experience_; }
int Player::gold() const noexcept { return gold_; }
int Player::wins() const noexcept { return wins_; }
bool Player::alive() const noexcept { return health_ > 0; }

int Player::attackDamage(Random& random) const {
    return random.between(12, 18) + (level_ - 1) * 3;
}

int Player::takeDamage(int amount) {
    if (amount <= 0 || health_ <= 0) {
        return 0;
    }

    int applied = amount;
    if (guarding_) {
        applied = (amount + 1) / 2;
        guarding_ = false;
    }
    applied = std::min(applied, health_);
    health_ -= applied;
    return applied;
}

void Player::raiseGuard() noexcept { guarding_ = true; }

bool Player::healWithPotion() {
    if (potions_ <= 0 || health_ >= maxHealth_) {
        return false;
    }
    --potions_;
    health_ = std::min(maxHealth_, health_ + 35);
    return true;
}

void Player::addItem(Item item, int quantity) {
    if (quantity < 0) {
        throw std::invalid_argument("item quantity cannot be negative");
    }
    if (item == Item::Potion) {
        potions_ += quantity;
    } else {
        bombs_ += quantity;
    }
}

bool Player::consumeItem(Item item) {
    int* count = item == Item::Potion ? &potions_ : &bombs_;
    if (*count <= 0) {
        return false;
    }
    --(*count);
    return true;
}

int Player::itemCount(Item item) const noexcept {
    return item == Item::Potion ? potions_ : bombs_;
}

void Player::addGold(int amount) noexcept {
    gold_ = std::max(0, gold_ + amount);
}

int Player::experienceNeededForLevel(int level) noexcept {
    return 75 + (level - 1) * 50;
}

void Player::grantExperience(int amount) {
    experience_ += std::max(0, amount);
    while (experience_ >= experienceNeededForLevel(level_)) {
        experience_ -= experienceNeededForLevel(level_);
        ++level_;
        maxHealth_ += 15;
        health_ = maxHealth_;
    }
}

void Player::recordWin() noexcept { ++wins_; }

PlayerSnapshot Player::snapshot() const {
    return {name_, health_, maxHealth_, level_, experience_, gold_, wins_, potions_, bombs_};
}

void Player::restore(const PlayerSnapshot& snapshot) {
    if (snapshot.name.empty() || snapshot.maxHealth <= 0 || snapshot.level <= 0 ||
        snapshot.health < 0 || snapshot.health > snapshot.maxHealth || snapshot.experience < 0 ||
        snapshot.gold < 0 || snapshot.wins < 0 || snapshot.potions < 0 || snapshot.bombs < 0) {
        throw std::invalid_argument("invalid player snapshot");
    }

    name_ = snapshot.name;
    health_ = snapshot.health;
    maxHealth_ = snapshot.maxHealth;
    level_ = snapshot.level;
    experience_ = snapshot.experience;
    gold_ = snapshot.gold;
    wins_ = snapshot.wins;
    potions_ = snapshot.potions;
    bombs_ = snapshot.bombs;
    guarding_ = false;
}

}  // namespace minigame
