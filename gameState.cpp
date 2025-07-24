#include "gameState.h"
#include "characters.h"
#include "input_utils.h"
#include "player.h"
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
    return generatedValue;
}

Items RandomGenerator::randomItem(){
    std::uniform_int_distribution<int> item(0, 3);
    int randomItem = item(m_engine);
    enum Items random = static_cast<Items>(randomItem);
    return random;
}

int RandomGenerator::randomHeal(){
    std::uniform_int_distribution<int> health(40, 100);
    int randomHealth = health(m_engine);
    return randomHealth;
}
int RandomGenerator::randomDamage(){
    std::uniform_int_distribution<int> damage(20, 50);
    int randomDamageCount = damage(m_engine);
        return randomDamageCount;
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