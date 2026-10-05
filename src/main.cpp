#include <iostream>

#include "minigame/game_state.h"

int main() {
    minigame::GameState game(std::cin, std::cout);
    return game.run();
}
