#ifndef MINIGAME_CHARACTER_H
#define MINIGAME_CHARACTER_H

#include <string>

namespace minigame {

class RandomSource;

class Character {
public:
    Character(std::string name, int maxHealth);
    virtual ~Character() = default;

    const std::string& name() const noexcept;
    int health() const noexcept;
    int maxHealth() const noexcept;
    bool alive() const noexcept;

    void takeDamage(int amount);
    int heal(int amount);

    virtual int attackDamage(RandomSource& random) const = 0;
    virtual int healAmount(RandomSource& random) const;

private:
    std::string name_;
    int health_;
    int maxHealth_;
};

class Knight final : public Character {
public:
    Knight();
    int attackDamage(RandomSource& random) const override;
};

class Mage final : public Character {
public:
    Mage();
    int attackDamage(RandomSource& random) const override;
    int healAmount(RandomSource& random) const override;
};

class Dragon final : public Character {
public:
    Dragon();
    int attackDamage(RandomSource& random) const override;
    int healAmount(RandomSource& random) const override;
};

}  // namespace minigame

#endif
