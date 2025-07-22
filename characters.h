#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <string>

class Player;
class RandomGenerator;

enum class ShieldState{
    down,
    up,
};
class Character{
    protected:
        std::string name;
        int health;
        ShieldState CharacterShield;
        RandomGenerator* randomNumber;

    public:
        Character();
        virtual void attack(Character* character) = 0;
        virtual std::string getName(Character* character) const;
        virtual int getHealth(Character* character) const;      
        virtual void modifyHealth(Character* character, int modifyValue); 
        virtual void attackPlayer(Player* player) = 0;
        void switchShieldState(); 
        virtual ShieldState getShieldState(Character* character) const; 
        virtual void defend() = 0;
        virtual void heal() = 0;
        virtual ~Character();
};

class Mage : public Character {
public:
    Mage();
    void attack(Character* character) override;
    void attackPlayer(Player* player) override;
    bool healthFull() const;
    void heal() override;
    void defend() override;
};

class Knight : public Character {
public:
    Knight();
    void attack(Character* character) override;
    void attackPlayer(Player* player) override;
    bool healthFull() const;
    void heal() override;
    void defend() override;
};

#endif
