#include "minigame/save_game.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace minigame {
namespace {
template <typename T>
bool readField(std::istream& input, const char* expectedKey, T& value) {
    std::string key;
    return static_cast<bool>(input >> key >> value) && key == expectedKey;
}
}  // namespace

bool SaveGame::save(const Campaign& campaign, const std::filesystem::path& path, std::string& error) {
    std::ofstream output(path, std::ios::trunc);
    if (!output) {
        error = "Could not open save file for writing.";
        return false;
    }

    const auto data = campaign.snapshot();
    output << "MINIGAME_SAVE_V2\n";
    output << "name " << std::quoted(data.player.name) << '\n';
    output << "health " << data.player.health << '\n';
    output << "max_health " << data.player.maxHealth << '\n';
    output << "level " << data.player.level << '\n';
    output << "experience " << data.player.experience << '\n';
    output << "gold " << data.player.gold << '\n';
    output << "wins " << data.player.wins << '\n';
    output << "potions " << data.player.potions << '\n';
    output << "bombs " << data.player.bombs << '\n';
    output << "explorations " << data.explorations << '\n';
    output << "bosses_defeated " << data.bossesDefeated << '\n';
    output << "next_boss_at_win " << data.nextBossAtWin << '\n';

    if (!output) {
        error = "Failed while writing save file.";
        return false;
    }
    error.clear();
    return true;
}

std::optional<CampaignSnapshot> SaveGame::load(const std::filesystem::path& path, std::string& error) {
    std::ifstream input(path);
    if (!input) {
        error = "No readable save file was found.";
        return std::nullopt;
    }

    std::string magic;
    std::getline(input, magic);
    if (magic != "MINIGAME_SAVE_V2") {
        error = "Unsupported or corrupt save format.";
        return std::nullopt;
    }

    CampaignSnapshot data;
    std::string key;
    if (!(input >> key) || key != "name" || !(input >> std::quoted(data.player.name)) ||
        !readField(input, "health", data.player.health) ||
        !readField(input, "max_health", data.player.maxHealth) ||
        !readField(input, "level", data.player.level) ||
        !readField(input, "experience", data.player.experience) ||
        !readField(input, "gold", data.player.gold) ||
        !readField(input, "wins", data.player.wins) ||
        !readField(input, "potions", data.player.potions) ||
        !readField(input, "bombs", data.player.bombs) ||
        !readField(input, "explorations", data.explorations) ||
        !readField(input, "bosses_defeated", data.bossesDefeated) ||
        !readField(input, "next_boss_at_win", data.nextBossAtWin)) {
        error = "Save file is incomplete or malformed.";
        return std::nullopt;
    }

    try {
        Campaign validation(data.player.name);
        validation.restore(data);
    } catch (const std::exception& ex) {
        error = std::string("Save validation failed: ") + ex.what();
        return std::nullopt;
    }

    error.clear();
    return data;
}

}  // namespace minigame
