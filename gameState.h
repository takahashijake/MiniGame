#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <cstdlib> 
#include <random>
#include <chrono> 
#include <iostream>
#include <thread>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <memory>

#include "characters.h"
#include "input_utils.cpp"
enum class Items{
    Sword,
    Potion,
    Key, 
    Gold,
};

void terminalClear(){
    std::cout << "\033[2J\033[1;1H" << std::endl;
}

class RandomGenerator {
public:

    RandomGenerator() {
        m_engine.seed(std::random_device{}());

       
    }
    int randomGenerator() {
        // Define a uniform integer distribution for the desired range.
        // The range [1, 100] means both 1 and 100 are possible outcomes.
        std::uniform_int_distribution<int> distribution(1, 100);

        // Generate a random number using the engine and the defined distribution,
        // and store it in a local variable.
        int generatedValue = distribution(m_engine);
        std::cout << "Random number is: " << generatedValue << std::endl;

        // Return the stored random number.
        return generatedValue;
    }

    Items randomItem(){
        std::uniform_int_distribution<int> item(0, 3);
        int randomItem = item(m_engine);
        enum Items random = static_cast<Items>(randomItem);
        return random;
    }

private:

    std::mt19937 m_engine;
};
class Player{

    private:
        int health;
        std::string name; 
        std::unordered_map<Items, int> inventory;

    public:

        Player(){
            std::cout << "Welcome to Jake Takahashi's minigame!" << std::endl;
            std::cout << "Before we start, what would you like your name to be: ";
            std::string tempName;
            std:getline(std::cin, tempName);
            name = tempName; 
            health = 1000;
            terminalClear();
        }

        void addInventory(Items thisItem, int quantity){
            inventory[thisItem] += quantity;
        }

        std::string inventoryString(Items thisItem){
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

        void viewInventory(){
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

        void attack(Character* character){
        }

    
};
class BattleSequence{
    private:
        Player* thisPlayer;
        Character* thisCharacter;

    public:
        BattleSequence(Player* playerArgument, Character* characterArgument){
            thisPlayer = playerArgument;
            thisCharacter = characterArgument;
        }

        void playerTurn(){
            std::cout << "It is your turn! What would you like to do?: " << std::endl;
            std::cout << "Press A to attack" << std::endl;
            setNonBlockingInput();
            char input_char = getNonBlockingChar();
            if (input_char != 0){
                input_char = toupper(input_char);
                if (input_char == 'A'){
                    std::cout << "Attack has been attempted!" << std::endl;
                }
            }
        }
        void mainBattle(){
            std::cout << "It is your turn! What would you to do?: " << std::endl;
            setNonBlockingInput();

        }

        ~BattleSequence(){
            delete thisPlayer;
            delete thisCharacter;
        }


};
class GameState{
    private:
        std::unique_ptr<RandomGenerator> RandomNumber;
        std::unique_ptr<Player> thisPlayer;
    public:

        GameState(){
            RandomNumber = std::make_unique<RandomGenerator>();
            thisPlayer = std::make_unique<Player>();

        }

        void gameAction1(){
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
                                    // You'd initiate a Battle sequence here, e.g.,
                                    // Battle currentBattle(*thisPlayer, std::move(newEnemy));
                                    // bool playerWon = currentBattle.startBattle();
                                }
                                else if (character <= 60){
                                    std::unique_ptr<Character> newEnemy = std::make_unique<Mage>();
                                    std::cout << "You have encountered a new enemy Mage! Prepare for battle!" << std::endl;
                                    std::this_thread::sleep_for(std::chrono::seconds(1));
                                    // Battle currentBattle(*thisPlayer, std::move(newEnemy));
                                    // bool playerWon = currentBattle.startBattle();
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

            restoreBlockingInput(); // Restore terminal settings when loop exits
        }

        void gameAction(){
            while (true ){
                std::cout << "To walk, please enter 1!" << std::endl;
                std::cout << "Please enter your option: ";
                int x;
                std::cin >> x;
                if (x == 1){
                    int chance = RandomNumber->randomGenerator();
                    if (chance <= 40){
                        int character = RandomNumber->randomGenerator();
                        if (character < 30){
                            std::unique_ptr<Character> newEnemy = std::make_unique<Knight>();
                            std::cout << "You have encountered a new enemy Knight! Prepare for battle!" << std::endl;
                            std::this_thread::sleep_for(std::chrono::seconds(2));
                            terminalClear();
                            continue;
                        }
                        else if (character <= 60){
                            std::unique_ptr<Character> newEnemy = std::make_unique<Mage>();
                            std::cout << "You have encountered a new enemey Mage! Prepare for battle!" << std::endl;
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            terminalClear();
                            continue;
                    }
                    }
                    else if (chance <= 60){
                        enum Items thisItem = RandomNumber->randomItem();
                        thisPlayer->addInventory(thisItem, 1);
                        thisPlayer->viewInventory();
                        terminalClear();
                        continue;
                    }      
                    else{
                        terminalClear();
                        continue;
                    }

                }
            }
    }
        void gameBegin(){
            std::cout << "Welcome to Jake Takahashi's minigame! Please enter what you would like to do! " << std::endl;
            std::cout << "To begin the game, please enter 1: " << std::endl;
            std::cout << "Please enter your option: ";
            int x;
            std::cin >> x;
            if (x == 1){
                    terminalClear();
                    gameAction1();
            }
        }
};



#endif 