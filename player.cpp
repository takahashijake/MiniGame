#include "player.h"
#include "characters.h"
#include "gameState.h"
void terminalClearing(){
    std::cout << "\033[2J\033[1;1H" << std::endl;
}

Player::Player(){
    std::cout << "Welcome to Jake Takahashi's minigame!" << std::endl;
    std::cout << "Before we start, what would you like your name to be: ";
    std::string tempName;
    std::getline(std::cin, tempName);
    name = tempName; 
    health = 1000;
    terminalClearing();
    randomNumber = new RandomGenerator();
}

void Player::addInventory(Items thisItem, int quantity){
    inventory[thisItem] += quantity;
}

void Player::subtractInventory(Items thisItem){
    inventory[thisItem]--;
}
int Player::getHealth(){
    return health; 
}

std::string Player::inventoryString(Items thisItem){
    switch(thisItem){
        case Items::Sword:
            return "Sword";
        case Items::Potion:
            return "Potion";
        case Items::Key:
            return "Key";
        case Items::Gold:
            return "Gold";
    }
}

 void Player::viewInventory(){
    if (inventory.empty()){
        std::cout << "Inventory is empty! " << std::endl;
    }
    else if (!inventory.empty()){
        typedef std::unordered_map<Items, int> InventoryMapType;
        for (InventoryMapType::iterator it = inventory.begin(); it != inventory.end(); ++it) {
            std::cout << "Number of " << inventoryString(it->first) << " is " << it->second << std::endl;
        }
    }
}

void Player::modifyHealth(int healthFactor){
    health = health - healthFactor;
}

bool Player::run(){
    int runChance = randomNumber->randomGenerator();
    if (runChance < 30){
        return true;
    }
    else{
        return false;
    }
    return false;
    
}

bool Player::inventoryContains(Items Items){
    return inventory.count(Items) > 0;
}

void Player::attack(Character* character){
    std::cout << "Attacked " << character->getName() << std::endl;
    if (character->getShieldState() == ShieldState::up){
        std::cout << "Attack failed! " << character->getName()  << " has shield up! " << std::endl;
    }
    else if (character->getShieldState() == ShieldState::down){
        int chance = randomNumber->randomGenerator();
        if (chance <= 30){
            std::cout << "Attack Missed" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        else if (chance > 30){
            int damage = randomNumber->randomGenerator();
            character->modifyHealth(damage);
            if (character->getHealth() < 0){

            }
            std::cout << character->getName() << " has " << character->getHealth() << " remaining!" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
}  

void Player::heal(){
    int randomhealth = randomNumber->randomHeal();
    Player::modifyHealth(randomhealth);
    Player::subtractInventory(Items::Potion);
}