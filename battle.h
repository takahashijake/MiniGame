#ifndef BATTLE_H
#define BATTLE_H

class Character;
class Player;
class RandomGenerator;

#include <memory>
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
        void enemyMove(Character* enemy, Player* player, turnState& turn);
        void mainBattle();
        ~BattleSequence();
};
#endif 