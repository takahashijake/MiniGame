#include "minigame/game_engine.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "minigame/items.h"

namespace minigame {
namespace {

std::uint32_t generateSeed() {
    std::random_device device;
    return device();
}

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
    : GameEngine(std::move(playerName), generateSeed()) {}

GameEngine::GameEngine(std::string playerName, std::uint32_t seed)
    : player_(std::move(playerName)),
      random_(std::make_unique<RandomGenerator>(seed)),
      seed_(seed) {
    setMessage("The road is open. Explore, gear up, and find the Dragon.");
}

GameEngine::GameEngine(std::string playerName, std::unique_ptr<RandomSource> random)
    : player_(std::move(playerName)), random_(std::move(random)) {
    if (!random_) {
        throw std::invalid_argument("GameEngine requires a random source");
    }
    setMessage("The road is open. Explore, gear up, and find the Dragon.");
}

void GameEngine::reset(std::string playerName) {
    resetSeeded(std::move(playerName), generateSeed());
}

void GameEngine::resetSeeded(std::string playerName, std::uint32_t seed) {
    player_ = Player(std::move(playerName));
    progression_ = Progression{};
    enemy_.reset();
    phase_ = GamePhase::Exploring;
    bossBattle_ = false;
    seed_ = seed;
    random_ = std::make_unique<RandomGenerator>(seed_);
    setMessage("A new adventure begins. Seed " + std::to_string(seed_) + ".");
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
    } else if (command == "buy:shield" || command == "shield") {
        buy(Item::Shield);
    } else if (command == "buy:key" || command == "key") {
        buy(Item::Key);
    } else if (command == "inventory" || command == "i") {
        std::ostringstream summary;
        summary << "Inventory: " << player_.itemCount(Item::Potion) << " potion(s), "
                << player_.itemCount(Item::Gold) << " gold";
        if (player_.hasItem(Item::Sword)) {
            summary << ", Sword";
        }
        if (player_.hasItem(Item::Shield)) {
            summary << ", Shield";
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
    result.defense = player_.defense();
    result.gold = player_.itemCount(Item::Gold);
    result.potions = player_.itemCount(Item::Potion);
    result.hasSword = player_.hasItem(Item::Sword);
    result.hasShield = player_.hasItem(Item::Shield);
    result.hasKey = player_.hasItem(Item::Key);
    result.gateOpened = progression_.gateOpened();
    result.victories = progression_.victories();
    result.seed = seed_;
    result.phase = phaseName(phase_);
    result.message = message_;
    result.bossAvailable = progression_.bossAvailable(player_);

    if (enemy_) {
        result.enemyName = enemy_->name();
        result.enemyHealth = enemy_->health();
        result.enemyMaxHealth = enemy_->maxHealth();
        result.enemyEnraged =
            bossBattle_ && enemy_->health() <= enemy_->maxHealth() / 2 && enemy_->alive();
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
         << "\"defense\":" << state.defense << ","
         << "\"gold\":" << state.gold << ","
         << "\"potions\":" << state.potions << ","
         << "\"hasSword\":" << jsonBool(state.hasSword) << ","
         << "\"hasShield\":" << jsonBool(state.hasShield) << ","
         << "\"hasKey\":" << jsonBool(state.hasKey) << ","
         << "\"gateOpened\":" << jsonBool(state.gateOpened) << ","
         << "\"victories\":" << state.victories << ","
         << "\"victoriesRequired\":" << state.victoriesRequired << ","
         << "\"seed\":" << state.seed << ","
         << "\"phase\":\"" << state.phase << "\","
         << "\"message\":\"" << escapeJson(state.message) << "\","
         << "\"enemyName\":\"" << escapeJson(state.enemyName) << "\","
         << "\"enemyHealth\":" << state.enemyHealth << ","
         << "\"enemyMaxHealth\":" << state.enemyMaxHealth << ","
         << "\"enemyEnraged\":" << jsonBool(state.enemyEnraged) << ","
         << "\"bossAvailable\":" << jsonBool(state.bossAvailable)
         << "}";
    return json.str();
}

std::string GameEngine::saveState() const {
    const std::string enemyName = enemy_ ? enemy_->name() : "";
    const int enemyHealth = enemy_ ? enemy_->health() : 0;

    std::ostringstream save;
    save << "MG2 "
         << seed_ << " "
         << std::quoted(player_.name()) << " "
         << player_.health() << " "
         << player_.itemCount(Item::Gold) << " "
         << player_.itemCount(Item::Potion) << " "
         << (player_.hasItem(Item::Sword) ? 1 : 0) << " "
         << (player_.hasItem(Item::Shield) ? 1 : 0) << " "
         << (player_.hasItem(Item::Key) ? 1 : 0) << " "
         << progression_.victories() << " "
         << (progression_.gateOpened() ? 1 : 0) << " "
         << (progression_.bossDefeated() ? 1 : 0) << " "
         << static_cast<int>(phase_) << " "
         << (bossBattle_ ? 1 : 0) << " "
         << std::quoted(enemyName) << " "
         << enemyHealth << " "
         << std::quoted(message_) << " "
         << std::quoted(random_->serializeState());
    return save.str();
}

bool GameEngine::loadState(const std::string& save) {
    std::istringstream input(save);

    std::string magic;
    std::uint32_t seed = 0;
    std::string playerName;
    int health = 0;
    int gold = 0;
    int potions = 0;
    int sword = 0;
    int shield = 0;
    int key = 0;
    int victories = 0;
    int gateOpened = 0;
    int bossDefeated = 0;
    int phaseValue = 0;
    int bossBattle = 0;
    std::string enemyName;
    int enemyHealth = 0;
    std::string message;
    std::string randomState;

    input >> magic >> seed >> std::quoted(playerName) >> health >> gold >> potions >> sword >>
        shield >> key >> victories >> gateOpened >> bossDefeated >> phaseValue >> bossBattle >>
        std::quoted(enemyName) >> enemyHealth >> std::quoted(message) >> std::quoted(randomState);

    if (input.fail() || magic != "MG2" || health < 0 || health > Player::kMaxHealth ||
        gold < 0 || potions < 0 || victories < 0 || phaseValue < 0 || phaseValue > 3) {
        return false;
    }

    Player restoredPlayer(playerName);
    restoredPlayer.takeDamage(Player::kMaxHealth - health);
    restoredPlayer.addItem(Item::Gold, gold);
    restoredPlayer.addItem(Item::Potion, potions);
    if (sword != 0) {
        restoredPlayer.addItem(Item::Sword);
    }
    if (shield != 0) {
        restoredPlayer.addItem(Item::Shield);
    }
    if (key != 0) {
        restoredPlayer.addItem(Item::Key);
    }

    std::unique_ptr<Character> restoredEnemy;
    if (!enemyName.empty()) {
        restoredEnemy = createEnemy(enemyName);
        if (!restoredEnemy || enemyHealth < 0 || enemyHealth > restoredEnemy->maxHealth()) {
            return false;
        }
        restoredEnemy->takeDamage(restoredEnemy->maxHealth() - enemyHealth);
    }

    const GamePhase restoredPhase = static_cast<GamePhase>(phaseValue);
    if (restoredPhase == GamePhase::Battle && !restoredEnemy) {
        return false;
    }
    if (restoredPhase != GamePhase::Battle && restoredEnemy) {
        return false;
    }

    auto restoredRandom = std::make_unique<RandomGenerator>(seed);
    if (!randomState.empty() && !restoredRandom->restoreState(randomState)) {
        return false;
    }

    player_ = std::move(restoredPlayer);
    progression_.restore(victories, gateOpened != 0, bossDefeated != 0);
    random_ = std::move(restoredRandom);
    enemy_ = std::move(restoredEnemy);
    phase_ = restoredPhase;
    bossBattle_ = bossBattle != 0;
    seed_ = seed;
    message_ = std::move(message);
    return true;
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

std::unique_ptr<Character> GameEngine::createEnemy(const std::string& name) {
    if (name == "Knight") {
        return std::make_unique<Knight>();
    }
    if (name == "Mage") {
        return std::make_unique<Mage>();
    }
    if (name == "Rogue") {
        return std::make_unique<Rogue>();
    }
    if (name == "Golem") {
        return std::make_unique<Golem>();
    }
    if (name == "Dragon") {
        return std::make_unique<Dragon>();
    }
    return nullptr;
}

void GameEngine::walk() {
    const int event = random_->between(1, 100);

    if (event <= 58) {
        const int encounter = random_->between(1, 100);
        if (encounter <= 34) {
            startBattle(std::make_unique<Knight>(), false);
        } else if (encounter <= 59) {
            startBattle(std::make_unique<Mage>(), false);
        } else if (encounter <= 81) {
            startBattle(std::make_unique<Rogue>(), false);
        } else {
            startBattle(std::make_unique<Golem>(), false);
        }
        return;
    }

    if (event <= 82) {
        handleLoot();
        return;
    }

    if (event <= 94) {
        const int restored = player_.heal(random_->between(15, 30));
        setMessage("You discover a roadside shrine and recover " + std::to_string(restored) +
                   " HP.");
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
    if ((item == Item::Sword || item == Item::Shield || item == Item::Key) &&
        player_.hasItem(item)) {
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
    if (bossBattle_) {
        setMessage("The Dragon seals the arena. There is no escape.");
        enemyTurn();
        return;
    }

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

    if (move > 72 && enemy_->health() < enemy_->maxHealth() && enemy_->name() != "Rogue") {
        const int restored = enemy_->heal(enemy_->healAmount(*random_));
        setMessage(message_ + " The " + enemy_->name() + " recovers " +
                   std::to_string(restored) + " HP.");
        return;
    }

    int rawDamage = enemy_->attackDamage(*random_);
    const bool enraged =
        bossBattle_ && enemy_->health() <= enemy_->maxHealth() / 2 && enemy_->alive();
    if (enraged) {
        rawDamage += 8;
    }

    const int appliedDamage = player_.receiveDamage(rawDamage);
    std::string response = message_ + " ";
    if (enraged) {
        response += "The enraged Dragon ";
    } else {
        response += "The " + enemy_->name() + " ";
    }
    response += "hits back for " + std::to_string(appliedDamage) + " damage";
    if (player_.defense() > 0) {
        response += " after Shield mitigation";
    }
    response += ".";
    setMessage(response);

    if (!player_.alive()) {
        phase_ = GamePhase::Defeat;
        enemy_.reset();
        bossBattle_ = false;
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

    if (loot <= 38) {
        player_.addItem(Item::Potion);
        setMessage("You find a Potion tucked beside the trail.");
        return;
    }

    if (loot <= 56 && !player_.hasItem(Item::Key) && !progression_.gateOpened()) {
        player_.addItem(Item::Key);
        setMessage("You discover an old iron Key.");
        return;
    }

    if (loot <= 70 && !player_.hasItem(Item::Sword)) {
        player_.addItem(Item::Sword);
        setMessage("You uncover a Sword. Your attacks now deal bonus damage.");
        return;
    }

    if (loot <= 82 && !player_.hasItem(Item::Shield)) {
        player_.addItem(Item::Shield);
        setMessage("You recover a Shield. Incoming attacks now deal 5 less damage.");
        return;
    }

    const int gold = random_->between(1, 5);
    player_.addItem(Item::Gold, gold);
    setMessage("You find " + std::to_string(gold) + " gold.");
}

void GameEngine::setMessage(std::string message) {
    message_ = std::move(message);
}

}  // namespace minigame
