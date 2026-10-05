#include "minigame/game_state.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <string>

namespace minigame {
namespace {

std::string healthBar(int health, int maxHealth) {
    constexpr int width = 20;
    const int filled =
        maxHealth > 0 ? std::clamp((health * width) / maxHealth, 0, width) : 0;
    return "[" + std::string(static_cast<std::size_t>(filled), '#') +
           std::string(static_cast<std::size_t>(width - filled), '-') + "]";
}

}  // namespace

GameState::GameState(std::istream& input, std::ostream& output)
    : input_(input), output_(output), engine_("Adventurer") {}

int GameState::run() {
    printBanner();

    output_ << "Name your adventurer [Adventurer]: ";
    std::string name;
    if (!std::getline(input_, name) || name.empty()) {
        name = "Adventurer";
    }
    engine_.reset(name);

    output_ << "\nGoal: win three battles, secure a Key, open the ancient gate, "
               "and defeat the Dragon.\n";

    while (running_) {
        const GameSnapshot state = engine_.snapshot();
        printHud(state);
        output_ << "\n" << state.message << "\n";

        if (state.phase == "victory") {
            output_ << "\n*** VICTORY — " << state.playerName
                    << " defeated the Dragon. ***\n";
            break;
        }

        if (state.phase == "defeat") {
            output_ << "\n*** GAME OVER ***\n";
            break;
        }

        if (state.phase == "battle") {
            runBattleTurn();
        } else {
            runExplorationTurn();
        }
    }

    return 0;
}

void GameState::printBanner() const {
    output_ << "========================================\n"
            << "              M I N I G A M E           \n"
            << "      Native CLI + WebAssembly core     \n"
            << "========================================\n";
}

void GameState::printHud(const GameSnapshot& state) const {
    output_ << "\n----------------------------------------\n"
            << state.playerName << "  " << healthBar(state.health, state.maxHealth) << " "
            << state.health << "/" << state.maxHealth << " HP\n"
            << "Gold: " << state.gold << "  |  Potions: " << state.potions
            << "  |  Victories: " << state.victories << "/" << state.victoriesRequired
            << "  |  Sword: " << (state.hasSword ? "yes" : "no")
            << "  |  Key: "
            << (state.gateOpened ? "used" : (state.hasKey ? "yes" : "no")) << "\n";

    if (!state.enemyName.empty()) {
        output_ << state.enemyName << "  "
                << healthBar(state.enemyHealth, state.enemyMaxHealth) << " "
                << state.enemyHealth << "/" << state.enemyMaxHealth << " HP\n";
    }
}

void GameState::runExplorationTurn() {
    output_ << "\n[W] Explore  [I] Inventory  [M] Merchant  [B] Boss gate  [Q] Quit\n> ";
    const char choice = readChoice();

    switch (choice) {
        case 'W':
            engine_.perform("walk");
            break;
        case 'I':
            engine_.perform("inventory");
            break;
        case 'M':
            runMerchant();
            break;
        case 'B':
            engine_.perform("boss");
            break;
        case 'Q':
            running_ = false;
            output_ << "Thanks for playing.\n";
            break;
        default:
            engine_.perform("unknown");
            break;
    }
}

void GameState::runBattleTurn() {
    output_ << "\n[A] Attack  [H] Potion  [R] Run\n> ";
    const char choice = readChoice();

    switch (choice) {
        case 'A':
            engine_.perform("attack");
            break;
        case 'H':
            engine_.perform("heal");
            break;
        case 'R':
            engine_.perform("run");
            break;
        default:
            engine_.perform("unknown");
            break;
    }
}

void GameState::runMerchant() {
    while (running_ && engine_.snapshot().phase == "exploring") {
        const GameSnapshot state = engine_.snapshot();
        output_ << "\n=== Travelling Merchant ===\n"
                << "Gold: " << state.gold << "\n"
                << "[P] Potion - 3 gold\n"
                << "[S] Sword  - 8 gold (+8 attack damage)\n"
                << "[K] Key    - 6 gold\n"
                << "[Q] Leave\n> ";

        const char choice = readChoice();
        if (choice == 'Q') {
            return;
        }

        switch (choice) {
            case 'P':
                engine_.perform("buy:potion");
                break;
            case 'S':
                engine_.perform("buy:sword");
                break;
            case 'K':
                engine_.perform("buy:key");
                break;
            default:
                engine_.perform("unknown");
                break;
        }

        output_ << engine_.snapshot().message << "\n";
    }
}

char GameState::readChoice() const {
    std::string line;
    if (!std::getline(input_, line) || line.empty()) {
        return '\0';
    }
    return static_cast<char>(std::toupper(static_cast<unsigned char>(line.front())));
}

}  // namespace minigame
