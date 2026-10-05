#ifndef MINIGAME_ITEMS_H
#define MINIGAME_ITEMS_H

namespace minigame {

enum class Item {
    Sword,
    Potion,
    Key,
    Gold,
};

inline const char* itemName(Item item) noexcept {
    switch (item) {
        case Item::Sword:
            return "Sword";
        case Item::Potion:
            return "Potion";
        case Item::Key:
            return "Key";
        case Item::Gold:
            return "Gold";
    }
    return "Unknown";
}

}  // namespace minigame

#endif
