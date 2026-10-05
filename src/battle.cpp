#include "minigame/battle.h"

#include <cctype>
#include <iostream>
#include <string>

#include "minigame/character.h"
#include "minigame/player.h"
#include "minigame/random.h"

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

}  // namespace

BattleSequence::BattleSequence(Player& player,
                               Character& enemy,
                               RandomSource& random,
                               std::istream& input,
                               std::ostream& output)
    : player_(player), enemy_(enemy), random_(random), input_(input), output_(output) {}

BattleResult BattleSequence::run() {
    output_ << "\n=== Encounter: " << enemy_.name() << " ===\n";

    while (player_.alive() && enemy_.alive()) {
        printStatus();

        if (const auto result = playerTurn()) {
            return *result;
        }

        if (const auto result = enemyTurn()) {
            return *result;
        }
    }

    return player_.alive() ? BattleResult::Victory : BattleResult::Defeat;
}

std::optional<BattleResult> BattleSequence::playerTurn() {
    while (true) {
        output_ << "[A]ttack  [H]eal  [R]un > ";
        const char choice = readChoice(input_);

        if (choice == 'A') {
            const int damage = player_.attackDamage(random_);
            enemy_.takeDamage(damage);
            output_ << "You hit the " << enemy_.name() << " for " << damage << " damage.\n";
            if (!enemy_.alive()) {
                output_ << "The " << enemy_.name() << " falls.\n";
                return BattleResult::Victory;
            }
            return std::nullopt;
        }

        if (choice == 'H') {
            if (!player_.hasItem(Item::Potion)) {
                output_ << "You do not have a potion.\n";
                continue;
            }

            const int before = player_.health();
            player_.usePotion(random_);
            output_ << "You drink a potion and recover " << player_.health() - before << " HP.\n";
            return std::nullopt;
        }

        if (choice == 'R' || choice == 'Q') {
            if (choice == 'Q' || random_.chance(35)) {
                output_ << "You escape the encounter.\n";
                return BattleResult::Fled;
            }

            output_ << "You fail to escape.\n";
            return std::nullopt;
        }

        output_ << "Choose A, H, or R.\n";
    }
}

std::optional<BattleResult> BattleSequence::enemyTurn() {
    const int move = random_.between(1, 100);

    if (move > 90 && enemy_.name() != "Dragon") {
        output_ << "The " << enemy_.name() << " retreats.\n";
        return BattleResult::EnemyFled;
    }

    if (move > 70 && enemy_.health() < enemy_.maxHealth()) {
        const int restored = enemy_.heal(enemy_.healAmount(random_));
        output_ << "The " << enemy_.name() << " recovers " << restored << " HP.\n";
        return std::nullopt;
    }

    const int damage = enemy_.attackDamage(random_);
    player_.takeDamage(damage);
    output_ << "The " << enemy_.name() << " hits you for " << damage << " damage.\n";

    if (!player_.alive()) {
        output_ << player_.name() << " has been defeated.\n";
        return BattleResult::Defeat;
    }

    return std::nullopt;
}

void BattleSequence::printStatus() const {
    output_ << player_.name() << " HP " << player_.health() << "/" << Player::kMaxHealth
            << "  |  " << enemy_.name() << " HP " << enemy_.health() << "/"
            << enemy_.maxHealth() << "\n";
}

}  // namespace minigame
