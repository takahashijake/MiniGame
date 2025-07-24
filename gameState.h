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

private:
    std::mt19937 m_engine;
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
        void playerMove(turnState& playerTurn);
        void enemyMove(turnState& playerTurn);
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