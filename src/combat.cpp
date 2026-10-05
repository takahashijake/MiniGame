#include "minigame/combat.hpp"

#include <sstream>

#include "minigame/player.hpp"
#include "minigame/random.hpp"

namespace minigame {
namespace {
std::string damageMessage(const std::string& actor, int damage, const std::string& target) {
    std::ostringstream out;
    out << actor << " deals " << damage << " damage to " << target << '.';
    return out.str();
}
}  // namespace

CombatEngine::CombatEngine(Player& player, Enemy enemy, Random& random)
    : player_(player), enemy_(std::move(enemy)), random_(random) {}

const Enemy& CombatEngine::enemy() const noexcept { return enemy_; }
BattleState CombatEngine::state() const noexcept { return state_; }

TurnResult CombatEngine::perform(PlayerAction action) {
    TurnResult result;
    result.state = state_;
    if (state_ != BattleState::Ongoing) {
        result.events.push_back("The battle is already over.");
        return result;
    }

    switch (action) {
        case PlayerAction::Attack: {
            const int damage = enemy_.takeDamage(player_.attackDamage(random_));
            result.events.push_back(damageMessage(player_.name(), damage, enemy_.name()));
            break;
        }
        case PlayerAction::Defend:
            player_.raiseGuard();
            result.events.push_back("You brace for the next attack. Incoming damage will be halved.");
            break;
        case PlayerAction::Heal:
            if (player_.healWithPotion()) {
                result.events.push_back("You drink a potion and recover up to 35 health.");
            } else {
                result.events.push_back("You cannot use a potion right now.");
            }
            break;
        case PlayerAction::Bomb:
            if (player_.consumeItem(Item::Bomb)) {
                const int damage = enemy_.takeDamage(42 + player_.level() * 3);
                result.events.push_back(damageMessage("Your bomb", damage, enemy_.name()));
            } else {
                result.events.push_back("You reach for a bomb, but your pack is empty.");
            }
            break;
        case PlayerAction::Run:
            if (!enemy_.isBoss() && random_.chance(40)) {
                state_ = BattleState::Escaped;
                result.state = state_;
                result.events.push_back("You escape before the enemy can react.");
                return result;
            }
            result.events.push_back(enemy_.isBoss() ? "There is no running from a boss."
                                                    : "You fail to escape.");
            break;
    }

    if (enemy_.defeated()) {
        state_ = BattleState::Victory;
        result.state = state_;
        result.events.push_back("Enemy defeated.");
        return result;
    }

    enemyTurn(result);
    result.state = state_;
    return result;
}

void CombatEngine::enemyTurn(TurnResult& result) {
    if (enemy_.kind() == EnemyKind::Mage && enemy_.health() < enemy_.maxHealth() / 2 &&
        random_.chance(28)) {
        const int restored = enemy_.heal(random_);
        std::ostringstream out;
        out << enemy_.name() << " restores " << restored << " health.";
        result.events.push_back(out.str());
        return;
    }

    int rawDamage = enemy_.attackDamage(random_);
    if (enemy_.isBoss() && random_.chance(20)) {
        rawDamage += 8;
        result.events.push_back("The dragon unleashes a burst of flame!");
    }
    const int damage = player_.takeDamage(rawDamage);
    result.events.push_back(damageMessage(enemy_.name(), damage, player_.name()));

    if (!player_.alive()) {
        state_ = BattleState::Defeat;
        result.events.push_back("You fall in battle.");
    }
}

}  // namespace minigame
