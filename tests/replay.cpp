#include <iostream>
#include "minigame/game_engine.h"

int main() {
    minigame::GameEngine game("Replay", 42u);
    for (int turn = 0; turn < 40; ++turn) {
        std::cout << game.stateJson() << '\n';
        game.perform(game.snapshot().phase == "battle" ? "attack" : "walk");
    }
}
