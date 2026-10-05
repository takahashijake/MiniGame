#ifndef MINIGAME_BATTLE_H
#define MINIGAME_BATTLE_H

#include <iosfwd>
#include <optional>

namespace minigame {

class Character;
class Player;
class RandomSource;

enum class BattleResult {
    Victory,
    Fled,
    Defeat,
    EnemyFled,
};

class BattleSequence {
public:
    BattleSequence(Player& player,
                   Character& enemy,
                   RandomSource& random,
                   std::istream& input,
                   std::ostream& output);

    BattleResult run();

private:
    std::optional<BattleResult> playerTurn();
    std::optional<BattleResult> enemyTurn();
    void printStatus() const;

    Player& player_;
    Character& enemy_;
    RandomSource& random_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace minigame

#endif
