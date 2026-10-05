#ifndef MINIGAME_PROGRESSION_H
#define MINIGAME_PROGRESSION_H

#include "minigame/items.h"

namespace minigame {

class Player;
class RandomSource;

struct BattleReward {
    int gold{0};
    bool potionDropped{false};
};

class Progression {
public:
    static constexpr int kVictoriesForBoss = 3;

    int victories() const noexcept;
    bool gateOpened() const noexcept;
    bool bossDefeated() const noexcept;

    BattleReward recordVictory(Player& player, RandomSource& random);

    int price(Item item) const noexcept;
    bool purchase(Player& player, Item item) const;

    bool bossAvailable(const Player& player) const;
    bool openBossGate(Player& player);
    void recordBossDefeat() noexcept;

private:
    int victories_{0};
    bool gateOpened_{false};
    bool bossDefeated_{false};
};

}  // namespace minigame

#endif
