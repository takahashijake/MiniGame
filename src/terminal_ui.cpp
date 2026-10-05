#include "minigame/terminal_ui.hpp"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "minigame/campaign.hpp"
#include "minigame/enemy.hpp"

namespace minigame {

TerminalUI::TerminalUI() {
    colorEnabled_ = std::getenv("NO_COLOR") == nullptr;
    clearEnabled_ = std::getenv("MINIGAME_NO_CLEAR") == nullptr;
}

void TerminalUI::clear() const {
    if (clearEnabled_) {
        std::cout << "\033[2J\033[H";
    } else {
        std::cout << "\n------------------------------------------------------------\n";
    }
}

std::string TerminalUI::color(std::string_view code, std::string_view text) const {
    if (!colorEnabled_) {
        return std::string(text);
    }
    return "\033[" + std::string(code) + "m" + std::string(text) + "\033[0m";
}

std::string TerminalUI::healthBar(int current, int maximum, int width) const {
    maximum = std::max(1, maximum);
    current = std::clamp(current, 0, maximum);
    const int filled = current * width / maximum;
    std::string bar = "[";
    bar.append(static_cast<std::size_t>(filled), '#');
    bar.append(static_cast<std::size_t>(width - filled), '.');
    bar += "]";
    return bar;
}

void TerminalUI::banner() const {
    std::cout << color("1;36", R"(
 __  __ _       _  ____                      
|  \/  (_)_ __ (_)/ ___| __ _ _ __ ___   ___
| |\/| | | '_ \| | |  _ / _` | '_ ` _ \ / _ \
| |  | | | | | | | |_| | (_| | | | | | |  __/
|_|  |_|_|_| |_|_|\____|\__,_|_| |_| |_|\___|
)") << '\n';
    std::cout << "A compact C++ terminal RPG • campaign edition\n\n";
}

void TerminalUI::world(const Campaign& campaign) const {
    const auto& player = campaign.player();
    std::cout << color("1;37", player.name()) << "  LV " << player.level() << "  "
              << healthBar(player.health(), player.maxHealth()) << ' ' << player.health() << '/'
              << player.maxHealth() << " HP\n";
    std::cout << "Gold: " << player.gold() << "  Wins: " << player.wins()
              << "  Bosses: " << campaign.bossesDefeated() << "  Explores: "
              << campaign.explorations() << "\n";
    std::cout << "Potions: " << player.itemCount(Item::Potion)
              << "  Bombs: " << player.itemCount(Item::Bomb) << "\n\n";
    std::cout << color("1;33", "[E]") << " Explore   " << color("1;33", "[I]")
              << " Inventory   " << color("1;33", "[S]") << " Save   "
              << color("1;33", "[L]") << " Load   " << color("1;33", "[Q]")
              << " Quit\n";
}

void TerminalUI::inventory(const Campaign& campaign) const {
    const auto& player = campaign.player();
    std::cout << color("1;35", "PACK") << "\n";
    std::cout << "  Potion x" << player.itemCount(Item::Potion) << "  — restore 35 HP\n";
    std::cout << "  Bomb   x" << player.itemCount(Item::Bomb) << "  — heavy combat damage\n";
    std::cout << "  Gold   " << player.gold() << "\n";
}

void TerminalUI::battle(const Campaign& campaign, const Enemy& enemy,
                        const std::vector<std::string>& events) const {
    const auto& player = campaign.player();
    std::cout << color(enemy.isBoss() ? "1;31" : "1;33", enemy.isBoss() ? "BOSS BATTLE" : "BATTLE")
              << "\n\n";
    std::cout << player.name() << "  " << healthBar(player.health(), player.maxHealth()) << ' '
              << player.health() << '/' << player.maxHealth() << " HP\n";
    std::cout << enemy.name() << "  " << healthBar(enemy.health(), enemy.maxHealth()) << ' '
              << enemy.health() << '/' << enemy.maxHealth() << " HP\n\n";
    for (const auto& event : events) {
        std::cout << " • " << event << '\n';
    }
    if (!events.empty()) {
        std::cout << '\n';
    }
    std::cout << color("1;33", "[A]") << " Attack  " << color("1;33", "[D]") << " Defend  "
              << color("1;33", "[H]") << " Heal  " << color("1;33", "[B]") << " Bomb  "
              << color("1;33", "[R]") << " Run\n";
}

void TerminalUI::message(std::string_view text) const { std::cout << text << '\n'; }

std::string TerminalUI::prompt(std::string_view text) const {
    std::cout << text;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

char TerminalUI::command(std::string_view text) const {
    const std::string line = prompt(text);
    if (line.empty()) {
        return '\0';
    }
    char c = line.front();
    if (c >= 'a' && c <= 'z') {
        c = static_cast<char>(c - 'a' + 'A');
    }
    return c;
}

void TerminalUI::pause() const {
    (void)prompt("\nPress Enter to continue...");
}

}  // namespace minigame
