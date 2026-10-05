#ifndef MINIGAME_GAME_STATE_H
#define MINIGAME_GAME_STATE_H

#include <iosfwd>

#include "minigame/game_engine.h"

namespace minigame {

class GameState {
public:
    GameState(std::istream& input, std::ostream& output);

    int run();

private:
    void printBanner() const;
    void printHud(const GameSnapshot& state) const;
    void runExplorationTurn();
    void runBattleTurn();
    void runMerchant();
    char readChoice() const;

    std::istream& input_;
    std::ostream& output_;
    GameEngine engine_;
    bool running_{true};
};

}  // namespace minigame

#endif
