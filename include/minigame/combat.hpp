#pragma once

#include <string>
#include <vector>

#include "minigame/enemy.hpp"

namespace minigame {

class Player;
class Random;

enum class PlayerAction { Attack, Defend, Heal, Bomb, Run };
enum class BattleState { Ongoing, Victory, Escaped, Defeat };

struct TurnResult {
    BattleState state{BattleState::Ongoing};
    std::vector<std::string> events;
};

class CombatEngine {
public:
    CombatEngine(Player& player, Enemy enemy, Random& random);

    const Enemy& enemy() const noexcept;
    BattleState state() const noexcept;
    TurnResult perform(PlayerAction action);

private:
    void enemyTurn(TurnResult& result);

    Player& player_;
    Enemy enemy_;
    Random& random_;
    BattleState state_{BattleState::Ongoing};
};

}  // namespace minigame
