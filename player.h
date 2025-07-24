#ifndef PLAYER_H
#define PLAYER_H

#include <unordered_map>
#include <string>

class RandomGenerator;
class Character;
enum class Items;

class Player{

    private:
        int health;
        std::string name; 
        std::unordered_map<Items, int> inventory;
        RandomGenerator* randomNumber;

    public:
        Player();
        void addInventory(Items thisItem, int quantity);
        int getHealth();
        std::string inventoryString(Items thisItem);
        void viewInventory();
        void modifyHealth(int healthFactor);
        void attack(Character* character);
        bool inventoryContains(Items thisItem);
        bool run();
};

#endif 
