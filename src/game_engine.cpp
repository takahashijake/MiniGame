#include "minigame/game_engine.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "minigame/items.h"

namespace minigame {
namespace {

std::string escapeJson(const std::string& value) {
    std::ostringstream escaped;
    for (const char ch : value) {
        switch (ch) {
            case '"':
                escaped << "\\\"";
                break;
            case '\\':
                escaped << "\\\\";
                break;
            case '\n':
                escaped << "\\n";
                break;
            case '\r':
                escaped << "\\r";
                break;
            case '\t':
                escaped << "\\t";
                break;
            default:
                escaped << ch;
                break;
        }
    }
    return escaped.str();
}

const char* jsonBool(bool value) {
    return value ? "true" : "false";
}

}  // namespace

GameEngine::GameEngine(std::string playerName)
    : GameEngine(std::move(playerName), std::make_unique<RandomGenerator>()) {}

GameEngine::GameEngine(std::string playerName, std::unique_ptr<RandomSource> random)
    : player_(std::move(playerName)), random_(std::move(random)) {
    if (!random_) {
        throw std::invalid_argument("GameEngine requires a random source");
    }
    setMessage("The road is open. Explore, gear up, and find the Dragon.");
}

void GameEngine::reset(std::string playerName) {
    player_ = Player(std::move(playerName));
    progression_ = Progression{};
    enemy_.reset();
    phase_ = GamePhase::Exploring;
    bossBattle_ = false;
    setMessage("A new adventure begins.");
}

void GameEngine::perform(const std::string& rawCommand) {
    const std::string command = normalizeCommand(rawCommand);

    if (phase_ == GamePhase::Victory || phase_ == GamePhase::Defeat) {
        setMessage("This run is complete. Start a new game to continue.");
        return;
    }

    if (phase_ == GamePhase::Battle) {
        if (command == "attack" || command == "a") {
            playerAttack();
        } else if (command == "heal" || command == "h") {
            playerHeal();
        } else if (command == "run" || command == "r" || command == "flee") {
            playerRun();
        } else {
            setMessage("You are in combat. Attack, heal, or run.");
        }
        return;
    }

    if (command == "walk" || command == "w" || command == "explore") {
        walk();
    } else if (command == "boss" || command == "b" || command == "gate") {
        challengeBoss();
    } else if (command == "buy:potion" || command == "potion") {
        buy(Item::Potion);
    } else if (command == "buy:sword" || command == "sword") {
        buy(Item::Sword);
    } else if (command == "buy:key" || command == "key") {
        buy(Item::Key);
    } else if (command == "inventory" || command == "i") {
        std::ostringstream summary;
        summary << "Inventory: " << player_.itemCount(Item::Potion) << " potion(s), "
                << player_.itemCount(Item::Gold) << " gold";
        if (player_.hasItem(Item::Sword)) {
            summary << ", Sword";
        }
        if (player_.hasItem(Item::Key)) {
            summary << ", Key";
        }
        summary << ".";
        setMessage(summary.str());
    } else {
        setMessage("Choose an exploration action, visit the merchant, or approach the boss gate.");
    }
}

GameSnapshot GameEngine::snapshot() const {
    GameSnapshot result;
    result.playerName = player_.name();
    result.health = player_.health();
    result.gold = player_.itemCount(Item::Gold);
    result.potions = player_.itemCount(Item::Potion);
    result.hasSword = player_.hasItem(Item::Sword);
    result.hasKey = player_.hasItem(Item::Key);
    result.gateOpened = progression_.gateOpened();
    result.victories = progression_.victories();
    result.phase = phaseName(phase_);
    result.message = message_;
    result.bossAvailable = progression_.bossAvailable(player_);

    if (enemy_) {
        result.enemyName = enemy_->name();
        result.enemyHealth = enemy_->health();
        result.enemyMaxHealth = enemy_->maxHealth();
    }

    return result;
}

std::string GameEngine::stateJson() const {
    const GameSnapshot state = snapshot();
    std::ostringstream json;
    json << "{"
         << "\"playerName\":\"" << escapeJson(state.playerName) << "\","
         << "\"health\":" << state.health << ","
         << "\"maxHealth\":" << state.maxHealth << ","
         << "\"gold\":" << state.gold << ","
         << "\"potions\":" << state.potions << ","
         << "\"hasSword\":" << jsonBool(state.hasSword) << ","
         << "\"hasKey\":" << jsonBool(state.hasKey) << ","
         << "\"gateOpened\":" << jsonBool(state.gateOpened) << ","
         << "\"victories\":" << state.victories << ","
         << "\"victoriesRequired\":" << state.victoriesRequired << ","
         << "\"phase\":\"" << state.phase << "\","
         << "\"message\":\"" << escapeJson(state.message) << "\","
         << "\"enemyName\":\"" << escapeJson(state.enemyName) << "\","
         << "\"enemyHealth\":" << state.enemyHealth << ","
         << "\"enemyMaxHealth\":" << state.enemyMaxHealth << ","
         << "\"bossAvailable\":" << jsonBool(state.bossAvailable)
         << "}";
    return json.str();
}

std::string GameEngine::normalizeCommand(std::string command) {
    command.erase(command.begin(),
                  std::find_if(command.begin(), command.end(), [](unsigned char ch) {
                      return !std::isspace(ch);
                  }));
    command.erase(std::find_if(command.rbegin(), command.rend(), [](unsigned char ch) {
                      return !std::isspace(ch);
                  }).base(),
                  command.end());
    std::transform(command.begin(), command.end(), command.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return command;
}

const char* GameEngine::phaseName(GamePhase phase) noexcept {
    switch (phase) {
        case GamePhase::Exploring:
            return "exploring";
        case GamePhase::Battle:
            return "battle";
        case GamePhase::Defeat:
            return "defeat";
        case GamePhase::Victory:
            return "victory";
    }
    return "unknown";
}

void GameEngine::walk() {
    const int event = random_->between(1, 100);

    if (event <= 55) {
        if (random_->chance(55)) {
            startBattle(std::make_unique<Knight>(), false);
        } else {
            startBattle(std::make_unique<Mage>(), false);
        }
        return;
    }

    if (event <= 85) {
        handleLoot();
        return;
    }

    setMessage("The trail is quiet. You make progress without incident.");
}

void GameEngine::challengeBoss() {
    if (progression_.victories() < Progression::kVictoriesForBoss) {
        const int remaining = Progression::kVictoriesForBoss - progression_.victories();
        setMessage("The gate rejects you. Win " + std::to_string(remaining) +
                   " more battle(s) first.");
        return;
    }

    if (!progression_.gateOpened() && !player_.hasItem(Item::Key)) {
        setMessage("The ancient gate is locked. Find or buy a Key.");
        return;
    }

    if (!progression_.openBossGate(player_)) {
        setMessage("The gate remains sealed.");
        return;
    }

    startBattle(std::make_unique<Dragon>(), true);
    setMessage("The ancient gate opens. The Dragon descends into the arena.");
}

void GameEngine::buy(Item item) {
    if (item == Item::Key && progression_.gateOpened()) {
        setMessage("The boss gate is already open. You do not need another Key.");
        return;
    }

    const int cost = progression_.price(item);
    if ((item == Item::Sword || item == Item::Key) && player_.hasItem(item)) {
        setMessage("You already own that item.");
        return;
    }

    if (player_.itemCount(Item::Gold) < cost) {
        setMessage("You need " + std::to_string(cost) + " gold to buy " + itemName(item) + ".");
        return;
    }

    if (progression_.purchase(player_, item)) {
        setMessage(std::string("Purchased ") + itemName(item) + ".");
        return;
    }

    setMessage("That purchase cannot be completed.");
}

void GameEngine::startBattle(std::unique_ptr<Character> enemy, bool bossBattle) {
    enemy_ = std::move(enemy);
    bossBattle_ = bossBattle;
    phase_ = GamePhase::Battle;
    setMessage("A " + enemy_->name() + " blocks your path.");
}

void GameEngine::playerAttack() {
    if (!enemy_) {
        phase_ = GamePhase::Exploring;
        setMessage("There is no enemy to attack.");
        return;
    }

    const int damage = player_.attackDamage(*random_);
    enemy_->takeDamage(damage);

    if (!enemy_->alive()) {
        setMessage("You strike for " + std::to_string(damage) + " damage and defeat the " +
                   enemy_->name() + ".");
        resolveVictory();
        return;
    }

    setMessage("You strike the " + enemy_->name() + " for " + std::to_string(damage) + " damage.");
    enemyTurn();
}

void GameEngine::playerHeal() {
    if (!player_.hasItem(Item::Potion)) {
        setMessage("You do not have a Potion.");
        return;
    }

    const int before = player_.health();
    player_.usePotion(*random_);
    const int restored = player_.health() - before;
    setMessage("You recover " + std::to_string(restored) + " HP.");
    enemyTurn();
}

void GameEngine::playerRun() {
    if (random_->chance(35)) {
        enemy_.reset();
        phase_ = GamePhase::Exploring;
        bossBattle_ = false;
        setMessage("You escape the encounter.");
        return;
    }

    setMessage("You fail to escape.");
    enemyTurn();
}

void GameEngine::enemyTurn() {
    if (!enemy_ || !enemy_->alive() || phase_ != GamePhase::Battle) {
        return;
    }

    const int move = random_->between(1, 100);

    if (move > 90 && !bossBattle_) {
        const std::string enemyName = enemy_->name();
        enemy_.reset();
        phase_ = GamePhase::Exploring;
        setMessage("The " + enemyName + " retreats before it can attack.");
        return;
    }

    if (move > 70 && enemy_->health() < enemy_->maxHealth()) {
        const int restored = enemy_->heal(enemy_->healAmount(*random_));
        setMessage(message_ + " The " + enemy_->name() + " recovers " +
                   std::to_string(restored) + " HP.");
        return;
    }

    const int damage = enemy_->attackDamage(*random_);
    player_.takeDamage(damage);
    setMessage(message_ + " The " + enemy_->name() + " hits back for " +
               std::to_string(damage) + " damage.");

    if (!player_.alive()) {
        phase_ = GamePhase::Defeat;
        setMessage(message_ + " Your adventure ends here.");
    }
}

void GameEngine::resolveVictory() {
    if (!enemy_) {
        return;
    }

    const std::string enemyName = enemy_->name();
    enemy_.reset();

    if (bossBattle_) {
        progression_.recordBossDefeat();
        bossBattle_ = false;
        phase_ = GamePhase::Victory;
        setMessage("The " + enemyName + " falls. The realm is safe.");
        return;
    }

    const BattleReward reward = progression_.recordVictory(player_, *random_);
    bossBattle_ = false;
    phase_ = GamePhase::Exploring;

    std::ostringstream summary;
    summary << "Victory! You earn " << reward.gold << " gold";
    if (reward.potionDropped) {
        summary << " and find a Potion";
    }
    summary << ".";
    setMessage(summary.str());
}

void GameEngine::handleLoot() {
    const int loot = random_->between(1, 100);

    if (loot <= 45) {
        player_.addItem(Item::Potion);
        setMessage("You find a Potion tucked beside the trail.");
        return;
    }

    if (loot <= 65 && !player_.hasItem(Item::Key) && !progression_.gateOpened()) {
        player_.addItem(Item::Key);
        setMessage("You discover an old iron Key.");
        return;
    }

    if (loot <= 75 && !player_.hasItem(Item::Sword)) {
        player_.addItem(Item::Sword);
        setMessage("You uncover a Sword. Your attacks now deal bonus damage.");
        return;
    }

    const int gold = random_->between(1, 4);
    player_.addItem(Item::Gold, gold);
    setMessage("You find " + std::to_string(gold) + " gold.");
}

void GameEngine::setMessage(std::string message) {
    message_ = std::move(message);
}

}  // namespace minigame
