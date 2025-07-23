#include "gameState.h"
#include "characters.h"
#include "input_utils.h"
#include <cstdlib> 
#include <random>
#include <chrono> 
#include <iostream>
#include <thread>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <memory>


void terminalClear(){
    std::cout << "\033[2J\033[1;1H" << std::endl;
}

RandomGenerator::RandomGenerator(){
    m_engine.seed(std::random_device{}());
}

int RandomGenerator::randomGenerator(){
    std::uniform_int_distribution<int> distribution(1, 100);

    int generatedValue = distribution(m_engine);
    std::cout << "Random number is: " << generatedValue << std::endl;

    return generatedValue;
}

Items RandomGenerator::randomItem(){
    std::uniform_int_distribution<int> item(0, 3);
    int randomItem = item(m_engine);
    enum Items random = static_cast<Items>(randomItem);
    return random;
}

int RandomGenerator::randomDamage(){
    std::uniform_int_distribution<int> damage(20, 50);
    int randomDamageCount = damage(m_engine);
        return randomDamageCount;
}

Player::Player(){
    std::cout << "Welcome to Jake Takahashi's minigame!" << std::endl;
    std::cout << "Before we start, what would you like your name to be: ";
    std::string tempName;
    std::getline(std::cin, tempName);
    name = tempName; 
    health = 1000;
    terminalClear();
    randomNumber = new RandomGenerator();
}

void Player::addInventory(Items thisItem, int quantity){
    inventory[thisItem] += quantity;
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

BattleSequence::BattleSequence(const std::unique_ptr<Player>& playerArgument, const std::unique_ptr<Character>& characterArgument){
    thisPlayer = playerArgument.get();
    thisCharacter = characterArgument.get();
    std::cout << "Battle Sequence initiated " << std::endl;
    randomNumber = new RandomGenerator();
}

void BattleSequence::enemyMove(turnState& playerTurn){  
    std::cout << "Enemy has moved!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    terminalClear();
    playerTurn = turnState::playerTurn;
}
void BattleSequence::playerMove(turnState& playerTurn) {
    setNonBlockingInput();
    try {
        terminalClear(); // Clear once at the start
        std::cout << "It is your turn! What would you like to do?\n";
        std::cout << "Press A to attack\n";
        while (true) {
            char input_char = getNonBlockingChar();
            if (input_char != 0) { // If a key was pressed
                input_char = toupper(input_char);
                if (input_char == 'A') {
                    thisPlayer->attack(thisCharacter);
                    if (thisCharacter->getHealth() <= 0){
                        playerTurn = turnState::gameOver;
                    }
                    playerTurn = turnState::enemyTurn;
                    return;
                }
                else if (input_char == 'R'){
                    bool runSuccess = thisPlayer->run();
                    if (runSuccess){
                        std::cout << "Successfully ran away! " << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        terminalClear();
                        playerTurn = turnState::gameOver;
                        return;
                    }
                    else{
                        std::cout << "Too slow! Could not run away! " << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        terminalClear();
                        playerTurn = turnState::enemyTurn;
                    }
                }
            }
            // Small delay to prevent CPU overuse
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    } catch (...) {
        restoreBlockingInput(); // Ensure terminal is restored
        throw; // Re-throw after cleanup
        return;
    }
    restoreBlockingInput();
    return;
}
void BattleSequence::mainBattle() {
    enum turnState myGame = turnState::playerTurn;
    while (myGame != turnState::gameOver){
        if (myGame == turnState::playerTurn){
            playerMove(myGame);
            if (myGame == turnState::gameOver){
                break;
            }
            terminalClear();
        }
        else if (myGame == turnState::enemyTurn){
            enemyMove(myGame);
            if (myGame == turnState::gameOver){
                break;
            }
            terminalClear();
        }
    }
}

BattleSequence::~BattleSequence(){

}

GameState::GameState(){
    RandomNumber = std::make_unique<RandomGenerator>();
    thisPlayer = std::make_unique<Player>();
}

void GameState::gameAction(){
            setNonBlockingInput();
            try {
                while (true) {
                    terminalClear(); // Clear screen at start of each frame/loop iteration

                    std::cout << "------------------------------------------" << std::endl;
                    std::cout << "Welcome!" << std::endl;
                    std::cout << "Press 'W' to Walk, 'I' to view Inventory, 'Q' to Quit." << std::endl;
                    std::cout << "------------------------------------------" << std::endl;

                    char input_char = getNonBlockingChar(); // Check for input non-blockingly

                    if (input_char != 0) { // If a key was pressed
                        input_char = toupper(input_char); // Convert to uppercase for consistent checking

                        if (input_char == 'W') {
                            std::cout << "You chose to walk!" << std::endl;
                            int chance = RandomNumber->randomGenerator();
                            if (chance <= 40){
                                int character = RandomNumber->randomGenerator();
                                if (character < 30){
                                    std::unique_ptr<Character> newEnemy = std::make_unique<Knight>();
                                    std::cout << "You have encountered a new enemy Knight! Prepare for battle!" << std::endl;
                                    std::this_thread::sleep_for(std::chrono::seconds(2));
                                    std::unique_ptr<BattleSequence> battleStart = std::make_unique<BattleSequence>(thisPlayer, newEnemy);
                                    battleStart->mainBattle();
                                }
                                else if (character <= 60){
                                    std::unique_ptr<Character> newEnemy = std::make_unique<Mage>();
                                    std::cout << "You have encountered a new enemy Mage! Prepare for battle!" << std::endl;
                                    std::this_thread::sleep_for(std::chrono::seconds(1));
                                    std::unique_ptr<BattleSequence> battleStart = std::make_unique<BattleSequence>(thisPlayer, newEnemy);
                                    battleStart->mainBattle();
                
                                }
                            }
                            else if (chance <= 60){
                                Items thisItem = RandomNumber->randomItem();
                                thisPlayer->addInventory(thisItem, 1);
                                std::cout << "You found a " << thisPlayer->inventoryString(thisItem) << "!" << std::endl;
                                std::this_thread::sleep_for(std::chrono::seconds(1)); // Pause to show message
                            }
                            else{
                                std::cout << "You walked and found nothing." << std::endl;
                                std::this_thread::sleep_for(std::chrono::seconds(1)); // Pause to show message
                            }
                        } else if (input_char == 'I') {
                            std::cout << "Viewing Inventory:" << std::endl;
                            thisPlayer->viewInventory();
                            std::this_thread::sleep_for(std::chrono::seconds(3)); // Pause to let user view inventory
                        } else if (input_char == 'Q') {
                            std::cout << "Exiting game..." << std::endl;
                            break; // Exit the loop
                        } else {
                            std::cout << "Invalid input. Press 'W', 'I', or 'Q'." << std::endl;
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                    } else {
                        // No key pressed, so do continuous background tasks or idle animation
                        // std::cout << "Waiting for input..." << std::endl; // For debugging, remove in final game
                        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Short delay to prevent busy-looping
                    }
                } // end while true
            } catch (...) {
                // Catch any unexpected exceptions to ensure terminal is restored
                restoreBlockingInput();
                throw; // Re-throw the exception
            }

            restoreBlockingInput(); // Restore terminal settings when loop exit
}

void GameState::gameBegin(){
            std::cout << "Welcome to Jake Takahashi's minigame! Please enter what you would like to do! " << std::endl;
            std::cout << "To begin the game, please enter 1: " << std::endl;
            std::cout << "Please enter your option: ";
            int x;
            std::cin >> x;
            if (x == 1){
                    terminalClear();
                    gameAction();
            }
        }
//end of gameState.cpp