#include "characters.h"
#include "gameState.h"


Character::Character() : name("CharacterName"), health(0), CharacterShield(ShieldState::down), randomNumber(nullptr) {

}

std::string Character::getName() const { return name; }
int Character::getHealth() const { return health; }
void Character::modifyHealth(int modifyValue) {
    health -= modifyValue;
}
ShieldState Character::getShieldState() const { return CharacterShield; }
void Character::switchShieldState() {
    if (CharacterShield == ShieldState::up) {
        std::cout << "Shield is up! Putting shield down!" << std::endl;
        CharacterShield = ShieldState::down;
    } else {
        std::cout << "Shield is down! Putting shield up!" << std::endl;
        CharacterShield = ShieldState::up;
    }
}
Character::~Character() {
    delete randomNumber; 
}

Mage::Mage() {
    health = 500;
    name = "Mage";
    CharacterShield = ShieldState::down;
    randomNumber = new RandomGenerator(); 
}

void Mage::attack(Character* character) {
    if (character->getShieldState() == ShieldState::up) {
        std::cout << "Attack failed! " << character->getName() << " has shield up" << std::endl;
        return;
    }
    if (character->getHealth() <= 0) {
        std::cout << character->getName() << " has already died! Cannot attack again!" << std::endl;
    } else {
        std::cout << "Mage is attacking " << character->getName() << std::endl;
        character->modifyHealth(80);
        if (character->getHealth() <= 0) {
            std::cout << character->getName() << " has died!" << std::endl;
        } else {
            std::cout << character->getName() << " has " << character->getHealth() << " health remaining " << std::endl;
        }
    }
    std::cout << std::endl;
}

void Mage::attackPlayer(Player* player) {
    int damage = randomNumber->randomDamage();
    std::cout << "Mage has attacked you! Dealing " << damage << " damage!" << std::endl;
    player->modifyHealth(damage);
    std::cout << "Player has " << player->getHealth() << " remaining!" << std::endl;
}

bool Mage::healthFull() const {
    return health == 500;
}

void Mage::setHealth(int healthSet){
    health = healthSet;
}

void Knight::setHealth(int healthSet){
    health = healthSet;
}

void Mage::heal() {
    if (!healthFull()) {
        std::cout << "Health is currently at " << health << "! Now healing..." << std::endl;
        health += 70;
        if (health > 500) health = 500; // Cap at max health
        std::cout << "Used a health potion! Health is now at " << health << std::endl;
    }
    std::cout << std::endl;
}

void Mage::defend() {
    switchShieldState();
    std::cout << std::endl;
}


Knight::Knight() {
    health = 300;
    name = "Knight";
    CharacterShield = ShieldState::down;
    randomNumber = new RandomGenerator(); 
}

void Knight::attack(Character* character) {
    if (character->getShieldState() == ShieldState::up) {
        std::cout << "Attack failed! " << character->getName() << " has shield up" << std::endl;
    } else { // Shield is down
        std::cout << "Knight is attacking " << character->getName() << " with his sword!" << std::endl;
        character->modifyHealth(50);
        if (character->getHealth() <= 0) {
            std::cout << character->getName() << " has died!" << std::endl;
        } else {
            std::cout << character->getName() << " has " << character->getHealth() << " health remaining " << std::endl;
        }
    }
    std::cout << std::endl;
}

void Knight::attackPlayer(Player* player) {
    int damage = randomNumber->randomDamage();
    std::cout << "Knight has attacked you! Dealing " << damage << " damage!" << std::endl;
    player->modifyHealth(damage);
    std::cout << "Player has " << player->getHealth() << " remaining!" << std::endl;
}

bool Knight::healthFull() const {
    return health == 300;
}

void Knight::heal() {
    if (!healthFull()) {
        std::cout << "Health is currently at " << health << "! Now healing..." << std::endl;
        health += 70;
        if (health > 300) { // Cap at max health
            health = 300;
        }
        std::cout << "Used a health potion! Health is now at " << health << std::endl;
    }
    std::cout << std::endl;
}

void Knight::defend() {
    switchShieldState();
    std::cout << std::endl;
}

//end of characters.cpp