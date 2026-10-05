#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "minigame/campaign.hpp"

namespace minigame {

class SaveGame {
public:
    static bool save(const Campaign& campaign, const std::filesystem::path& path,
                     std::string& error);
    static std::optional<CampaignSnapshot> load(const std::filesystem::path& path,
                                                 std::string& error);
};

}  // namespace minigame
