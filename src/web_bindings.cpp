#include <emscripten/bind.h>

#include <string>

#include "minigame/game_engine.h"

EMSCRIPTEN_BINDINGS(minigame_module) {
    emscripten::class_<minigame::GameEngine>("GameEngine")
        .constructor<std::string>()
        .function("perform", &minigame::GameEngine::perform)
        .function("reset", &minigame::GameEngine::reset)
        .function("stateJson", &minigame::GameEngine::stateJson);
}
