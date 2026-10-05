#pragma once

#include <string_view>

namespace minigame {

enum class Item { Potion, Bomb };

enum class EnemyKind { Knight, Mage, Berserker, Dragon };

constexpr std::string_view toString(Item item) {
    switch (item) {
        case Item::Potion: return "Potion";
        case Item::Bomb: return "Bomb";
    }
    return "Unknown";
}

constexpr std::string_view toString(EnemyKind kind) {
    switch (kind) {
        case EnemyKind::Knight: return "Knight";
        case EnemyKind::Mage: return "Mage";
        case EnemyKind::Berserker: return "Berserker";
        case EnemyKind::Dragon: return "Dragon";
    }
    return "Unknown";
}

}  // namespace minigame
