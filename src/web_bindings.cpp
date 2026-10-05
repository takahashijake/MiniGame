#include <emscripten/bind.h>

#include <cstdint>
#include <string>

#include "minigame/game_engine.h"

EMSCRIPTEN_BINDINGS(minigame_module) {
    emscripten::class_<minigame::GameEngine>("GameEngine")
        .constructor<std::string>()
        .constructor<std::string, std::uint32_t>()
        .function("perform", &minigame::GameEngine::perform)
        .function("reset", &minigame::GameEngine::reset)
        .function("resetSeeded", &minigame::GameEngine::resetSeeded)
        .function("stateJson", &minigame::GameEngine::stateJson)
        .function("saveState", &minigame::GameEngine::saveState)
        .function("loadState", &minigame::GameEngine::loadState);
}
