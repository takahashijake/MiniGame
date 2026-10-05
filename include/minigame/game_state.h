#ifndef MINIGAME_GAME_STATE_H
#define MINIGAME_GAME_STATE_H

#include <iosfwd>
#include <memory>

#include "minigame/player.h"
#include "minigame/progression.h"
#include "minigame/random.h"

namespace minigame {

class Character;

class GameState {
public:
    GameState(std::istream& input, std::ostream& output);

    int run();

private:
    void printBanner() const;
    void printHud() const;
    void printMenu() const;
    void walk();
    void showInventory() const;
    void visitMerchant();
    void challengeBoss();
    void handleLoot();
    void startBattle(std::unique_ptr<Character> enemy, bool bossBattle);

    std::istream& input_;
    std::ostream& output_;
    RandomGenerator random_;
    Player player_;
    Progression progression_;
    bool running_{true};
};

}  // namespace minigame

#endif
