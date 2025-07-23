#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <cstdlib> 
#include <random>
#include <chrono> 
#include <iostream>
#include <thread>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <memory>

#include "characters.h"
enum class Items{
    Sword,
    Potion,
    Key,
    Gold
};

class RandomGenerator {
public:
    RandomGenerator();
    int randomGenerator();
    Items randomItem();
    int randomDamage();

private:
    std::mt19937 m_engine;
};

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
};

class BattleSequence{
    private:
        Character* thisCharacter;
        Player* thisPlayer;
        enum turnState{
            playerTurn,
            enemyTurn,
            gameOver
        };
        RandomGenerator* randomNumber;

    public:
        BattleSequence(const std::unique_ptr<Player>& playerArgument, const std::unique_ptr<Character>& characterArgument);
        void playerMove();
        void mainBattle();
        ~BattleSequence();
};
class GameState{
    private:
        std::unique_ptr<RandomGenerator> RandomNumber;
        std::unique_ptr<Player> thisPlayer;
    public:

        GameState();
        void gameAction();
        void gameBegin();
};



#endif 