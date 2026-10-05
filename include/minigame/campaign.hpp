#pragma once

#include <optional>
#include <string>

#include "minigame/enemy.hpp"
#include "minigame/player.hpp"

namespace minigame {

class Random;

enum class ExploreType { Battle, Boss, Loot, Quiet };

struct ExploreOutcome {
    ExploreType type{ExploreType::Quiet};
    std::string message;
    std::optional<Enemy> enemy;
    std::optional<Item> item;
    int goldFound{0};
};

struct CampaignSnapshot {
    PlayerSnapshot player;
    int explorations{0};
    int bossesDefeated{0};
    int nextBossAtWin{3};
};

class Campaign {
public:
    explicit Campaign(std::string playerName = "Adventurer");

    Player& player() noexcept;
    const Player& player() const noexcept;
    int explorations() const noexcept;
    int bossesDefeated() const noexcept;
    int nextBossAtWin() const noexcept;

    ExploreOutcome explore(Random& random);
    void resolveVictory(const Enemy& enemy);

    CampaignSnapshot snapshot() const;
    void restore(const CampaignSnapshot& snapshot);

private:
    Player player_;
    int explorations_{0};
    int bossesDefeated_{0};
    int nextBossAtWin_{3};
};

}  // namespace minigame
