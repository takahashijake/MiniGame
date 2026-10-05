#ifndef MINIGAME_GAME_ENGINE_H
#define MINIGAME_GAME_ENGINE_H

#include <cstdint>
#include <memory>
#include <string>

#include "minigame/character.h"
#include "minigame/player.h"
#include "minigame/progression.h"
#include "minigame/random.h"

namespace minigame {

enum class GamePhase {
    Exploring,
    Battle,
    Defeat,
    Victory,
};

struct GameSnapshot {
    std::string playerName;
    int health{0};
    int maxHealth{Player::kMaxHealth};
    int defense{0};
    int gold{0};
    int potions{0};
    bool hasSword{false};
    bool hasShield{false};
    bool hasKey{false};
    bool gateOpened{false};
    int victories{0};
    int victoriesRequired{Progression::kVictoriesForBoss};
    std::uint32_t seed{0};
    std::string phase;
    std::string message;
    std::string enemyName;
    int enemyHealth{0};
    int enemyMaxHealth{0};
    bool enemyEnraged{false};
    bool bossAvailable{false};
};

class GameEngine {
public:
    explicit GameEngine(std::string playerName = "Adventurer");
    GameEngine(std::string playerName, std::uint32_t seed);
    GameEngine(std::string playerName, std::unique_ptr<RandomSource> random);

    void reset(std::string playerName = "Adventurer");
    void resetSeeded(std::string playerName, std::uint32_t seed);
    void perform(const std::string& command);

    GameSnapshot snapshot() const;
    std::string stateJson() const;
    std::string saveState() const;
    bool loadState(const std::string& save);

private:
    static std::string normalizeCommand(std::string command);
    static const char* phaseName(GamePhase phase) noexcept;
    static std::unique_ptr<Character> createEnemy(const std::string& name);

    void walk();
    void challengeBoss();
    void buy(Item item);
    void startBattle(std::unique_ptr<Character> enemy, bool bossBattle);
    void playerAttack();
    void playerHeal();
    void playerRun();
    void enemyTurn();
    void resolveVictory();
    void handleLoot();
    void setMessage(std::string message);

    Player player_;
    Progression progression_;
    std::unique_ptr<RandomSource> random_;
    std::unique_ptr<Character> enemy_;
    GamePhase phase_{GamePhase::Exploring};
    bool bossBattle_{false};
    std::uint32_t seed_{0};
    std::string message_;
};

}  // namespace minigame

#endif
