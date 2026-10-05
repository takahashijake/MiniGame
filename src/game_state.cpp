#include "minigame/game_state.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "minigame/battle.h"
#include "minigame/character.h"
#include "minigame/items.h"

namespace minigame {
namespace {

char readChoice(std::istream& input) {
    std::string line;
    if (!std::getline(input, line)) {
        return 'Q';
    }
    if (line.empty()) {
        return '\0';
    }
    return static_cast<char>(std::toupper(static_cast<unsigned char>(line.front())));
}

std::string healthBar(int health) {
    constexpr int width = 20;
    const int filled = std::clamp((health * width) / Player::kMaxHealth, 0, width);
    return "[" + std::string(static_cast<std::size_t>(filled), '#') +
           std::string(static_cast<std::size_t>(width - filled), '-') + "]";
}

}  // namespace

GameState::GameState(std::istream& input, std::ostream& output)
    : input_(input), output_(output), player_("Adventurer") {}

int GameState::run() {
    printBanner();

    output_ << "Name your adventurer [Adventurer]: ";
    std::string name;
    if (std::getline(input_, name) && !name.empty()) {
        player_.setName(name);
    }

    output_ << "\nGoal: win " << Progression::kVictoriesForBoss
            << " battles, obtain a Key, open the ancient gate, and defeat the Dragon.\n";

    while (running_ && player_.alive() && !progression_.bossDefeated()) {
        printHud();
        printMenu();
        output_ << "> ";

        switch (readChoice(input_)) {
            case 'W':
                walk();
                break;
            case 'I':
                showInventory();
                break;
            case 'M':
                visitMerchant();
                break;
            case 'B':
                challengeBoss();
                break;
            case 'Q':
                running_ = false;
                output_ << "Thanks for playing.\n";
                break;
            default:
                output_ << "Unknown command. Choose W, I, M, B, or Q.\n";
                break;
        }
    }

    if (!player_.alive()) {
        output_ << "\nGame over. The road claims another adventurer.\n";
    } else if (progression_.bossDefeated()) {
        output_ << "\n*** Victory! " << player_.name()
                << " defeated the Dragon and completed the adventure. ***\n";
    }

    return 0;
}

void GameState::printBanner() const {
    output_ << "========================================\n"
            << "              M I N I G A M E           \n"
            << "        A terminal adventure in C++     \n"
            << "========================================\n";
}

void GameState::printHud() const {
    output_ << "\n----------------------------------------\n";
    output_ << player_.name() << "  " << healthBar(player_.health()) << " " << player_.health()
            << "/" << Player::kMaxHealth << " HP\n";
    output_ << "Gold: " << player_.itemCount(Item::Gold)
            << "  |  Victories: " << progression_.victories() << "/"
            << Progression::kVictoriesForBoss
            << "  |  Sword: " << (player_.hasItem(Item::Sword) ? "yes" : "no")
            << "  |  Key: "
            << (progression_.gateOpened() ? "used" : (player_.hasItem(Item::Key) ? "yes" : "no"))
            << "\n";
}

void GameState::printMenu() const {
    output_ << "[W] Walk   [I] Inventory   [M] Merchant   [B] Boss gate   [Q] Quit\n";
}

void GameState::walk() {
    output_ << "\nYou follow the road into the wilds...\n";
    const int event = random_.between(1, 100);

    if (event <= 55) {
        std::unique_ptr<Character> enemy;
        if (random_.chance(55)) {
            enemy = std::make_unique<Knight>();
        } else {
            enemy = std::make_unique<Mage>();
        }
        startBattle(std::move(enemy), false);
        return;
    }

    if (event <= 85) {
        handleLoot();
        return;
    }

    output_ << "The road is quiet. You make progress without incident.\n";
}

void GameState::showInventory() const {
    output_ << "\n=== Inventory ===\n";
    const std::array<Item, 4> items{Item::Sword, Item::Potion, Item::Key, Item::Gold};

    bool any = false;
    for (const Item item : items) {
        const int quantity = player_.itemCount(item);
        if (quantity > 0) {
            output_ << std::left << std::setw(8) << itemName(item) << " x" << quantity << "\n";
            any = true;
        }
    }

    if (!any) {
        output_ << "(empty)\n";
    }
}

void GameState::visitMerchant() {
    while (true) {
        output_ << "\n=== Travelling Merchant ===\n"
                << "Gold: " << player_.itemCount(Item::Gold) << "\n"
                << "[P] Potion - 3 gold\n"
                << "[S] Sword  - 8 gold (permanent +8 attack damage)\n"
                << "[K] Key    - 6 gold (opens the boss gate)\n"
                << "[Q] Leave merchant\n"
                << "> ";

        const char choice = readChoice(input_);
        if (choice == 'Q') {
            return;
        }

        Item item;
        if (choice == 'P') {
            item = Item::Potion;
        } else if (choice == 'S') {
            item = Item::Sword;
        } else if (choice == 'K') {
            item = Item::Key;
        } else {
            output_ << "The merchant does not recognize that request.\n";
            continue;
        }

        const int cost = progression_.price(item);
        if ((item == Item::Sword || item == Item::Key) && player_.hasItem(item)) {
            output_ << "You already own that item.\n";
            continue;
        }
        if (player_.itemCount(Item::Gold) < cost) {
            output_ << "You need " << cost << " gold for that item.\n";
            continue;
        }

        if (progression_.purchase(player_, item)) {
            output_ << "Purchased " << itemName(item) << ".\n";
        }
    }
}

void GameState::challengeBoss() {
    if (progression_.victories() < Progression::kVictoriesForBoss) {
        output_ << "The gate rejects you. Win "
                << Progression::kVictoriesForBoss - progression_.victories()
                << " more battle(s) first.\n";
        return;
    }

    if (!progression_.gateOpened() && !player_.hasItem(Item::Key)) {
        output_ << "The ancient gate is locked. Find or buy a Key.\n";
        return;
    }

    if (!progression_.openBossGate(player_)) {
        output_ << "The gate remains sealed.\n";
        return;
    }

    output_ << "\nThe Key turns. The ancient gate opens, revealing the Dragon.\n";
    startBattle(std::make_unique<Dragon>(), true);
}

void GameState::handleLoot() {
    const int loot = random_.between(1, 100);

    if (loot <= 45) {
        player_.addItem(Item::Potion);
        output_ << "You find a Potion tucked beside the trail.\n";
        return;
    }

    if (loot <= 65 && !player_.hasItem(Item::Key) && !progression_.gateOpened()) {
        player_.addItem(Item::Key);
        output_ << "You discover an old iron Key. It may open something important.\n";
        return;
    }

    if (loot <= 75 && !player_.hasItem(Item::Sword)) {
        player_.addItem(Item::Sword);
        output_ << "You uncover a Sword. Your attacks will now deal bonus damage.\n";
        return;
    }

    const int gold = random_.between(1, 4);
    player_.addItem(Item::Gold, gold);
    output_ << "You find " << gold << " gold.\n";
}

void GameState::startBattle(std::unique_ptr<Character> enemy, bool bossBattle) {
    BattleSequence battle(player_, *enemy, random_, input_, output_);
    const BattleResult result = battle.run();

    if (result == BattleResult::Victory) {
        if (bossBattle) {
            progression_.recordBossDefeat();
            return;
        }

        const BattleReward reward = progression_.recordVictory(player_, random_);
        output_ << "Victory reward: " << reward.gold << " gold";
        if (reward.potionDropped) {
            output_ << " and a Potion";
        }
        output_ << ".\n";
        return;
    }

    if (result == BattleResult::Defeat) {
        running_ = false;
        return;
    }

    if (result == BattleResult::EnemyFled) {
        output_ << "The encounter ends without a victory reward.\n";
    }
}

}  // namespace minigame
