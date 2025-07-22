#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <iostream> 
#include <memory>
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
        enum ShieldState CharacterShield;
        RandomGenerator* randomNumber;

    public:
        Character(){
            name = "CharacterName";
            health = 0;
            CharacterShield = ShieldState::down;
        }
        virtual void attack(Character* character) = 0;
        virtual std::string getName(Character* character){return character->name;}
        virtual int getHealth(Character* character){return character->health;}
        virtual void modifyHealth(Character* character, int modifyValue){
            health = health - modifyValue;
        }
        virtual void attackPlayer(Player* player);
        void switchShieldState(Character* character){
            if (character->CharacterShield == ShieldState::up){
                std::cout << "Shield is up! Putting shield down!" << std::endl;
                character->CharacterShield = ShieldState::down;
            }
            else if (character->CharacterShield == ShieldState::down){
                std::cout << "Shield is down! Putting shield up!" << std::endl;
                character->CharacterShield = ShieldState::up;
            }
        }
        void playerFunction(Player& thisPlayer){
            thisPlayer.modifyHealth(int healthFactor);
        }
        virtual ShieldState getShieldState(Character* character){return character->CharacterShield;}
        virtual void defend() = 0;
        virtual void heal() = 0;

        virtual ~Character(){

        }
};

class Mage : public Character{

    public:

        Mage(){
            health = 500;
            name = "Mage";
            CharacterShield = ShieldState::down;
            randomNumber = new RandomGenerator();

        }

        void attack(Character* character) override{
            if (getShieldState(character) == ShieldState::up){
                std::cout << "Attack failed! " << getName(character) << " has shield up" << std::endl;
                return;
            }
            if (getHealth(character) <= 0){
                std::cout << getName(character) << " has already died! Cannot attack again!" << std::endl;
            }
            else if (getShieldState(character) == ShieldState::down){
                std::cout << "Mage is attacking " << getName(character) << std::endl;
                modifyHealth(character, 80);
                if (getHealth(character) <= 0){
                    std::cout << getName(character) << " has died!" << std::endl;
                }
                else{
                    std::cout << getName(character) << " has " << getHealth(character) << " health remaining " << std::endl;
                }
            }
            std::cout << std::endl;
        }

        void attackPlayer(Player* player) override{
            int damage = randomNumber->randomDamage();
            std::cout << "Mage has attacked you! Dealing " << damage << " damage!" << std::endl;
            player->modifyHealth(damage);
            std::cout << "Player has " << player->getHealth() << " remaining!" << std::endl;
        }

        bool healthFull(){
            if (health == 500){
                return true;
            }
            else{
                return false;
            }
            return false;
        }

        void heal() override{
            if (!healthFull()){
                std::cout << "Health is currently at " << health << "! Now healing..." << std::endl;
                health = health + 70;
                std::cout << "Used a health potion! Health is now at " << health << std::endl;
            }
            std::cout << std::endl;
        }

        void defend() override{
            switchShieldState(this);
            std::cout << std::endl;
        }

};

class Knight: public Character{
    
    public:
        Knight(){
            health = 300;
            name = "Knight";
            CharacterShield = ShieldState::down;
        }

        void attack(Character* character) override{
            if (getShieldState(character) == ShieldState::up){
                std::cout << "Attack failed! " << getName(character) << " has shield up" << std::endl;
            }
            else if (getShieldState(character) == ShieldState::down){
                std::cout << "Knight is attacking " << getName(character) << " with his sword!" << std::endl;
                modifyHealth(character, 50);
                
            }
            std::cout << std::endl;
        }

        bool healthFull(){
            if (health == 500){
                return true;
            }
            else{
                return false;
            }
            return false;
        }

        void heal() override{
            if (!healthFull()){
                std::cout << "Health is currently at " << health << "! Now healing..." << std::endl;
                health = health + 70;
                std::cout << "Used a health potion! Health is now at " << health << std::endl;
            }
            std::cout << std::endl;
        }

        void defend() override{
            switchShieldState(this);
            std::cout << std::endl;
        }
};

#endif 