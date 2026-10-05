#include "minigame/character.h"

#include <algorithm>
#include <utility>

#include "minigame/random.h"

namespace minigame {

Character::Character(std::string name, int maxHealth)
    : name_(std::move(name)), health_(maxHealth), maxHealth_(maxHealth) {}

const std::string& Character::name() const noexcept {
    return name_;
}

int Character::health() const noexcept {
    return health_;
}

int Character::maxHealth() const noexcept {
    return maxHealth_;
}

bool Character::alive() const noexcept {
    return health_ > 0;
}

void Character::takeDamage(int amount) {
    health_ = std::max(0, health_ - std::max(0, amount));
}

int Character::heal(int amount) {
    const int previous = health_;
    health_ = std::min(maxHealth_, health_ + std::max(0, amount));
    return health_ - previous;
}

int Character::healAmount(RandomSource& random) const {
    return random.between(12, 22);
}

Knight::Knight() : Character("Knight", 90) {}

int Knight::attackDamage(RandomSource& random) const {
    return random.between(10, 20);
}

Mage::Mage() : Character("Mage", 75) {}

int Mage::attackDamage(RandomSource& random) const {
    return random.between(11, 22);
}

int Mage::healAmount(RandomSource& random) const {
    return random.between(18, 30);
}

Dragon::Dragon() : Character("Dragon", 160) {}

int Dragon::attackDamage(RandomSource& random) const {
    return random.between(18, 30);
}

int Dragon::healAmount(RandomSource& random) const {
    return random.between(20, 34);
}

}  // namespace minigame
