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
class Player;
void terminalClear();
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
    int randomHeal();
    int randomMove();

private:
    std::mt19937 m_engine;
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