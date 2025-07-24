#include "battle.h"
#include "characters.h"
#include "gameState.h"
#include "input_utils.h"
#include "player.h"
#include <iostream>
#include <chrono>

void terminalClear(){
    std::cout << "\033[2J\033[1;1H" << std::endl;
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
        std::cout << "Press A to attack, R to run away, H to heal " << std::endl;
        while (true) {
            char input_char = getNonBlockingChar();
            if (input_char != 0) { // If a key was pressed
                input_char = toupper(input_char);
                if (input_char == 'A') {
                    terminalClear();
                    thisPlayer->attack(thisCharacter);
                    if (thisCharacter->getHealth() <= 0){
                        thisCharacter->setHealth(0);
                        playerTurn = turnState::gameOver;
                        terminalClear();
                        std::cout << "Congragulations! You have defeated the " << thisCharacter->getName() << std::endl;
                        return;
                    }
                    playerTurn = turnState::enemyTurn;
                    return;
                }
                else if (input_char == 'R'){
                    terminalClear();
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
                        return;
                    }
                }
                else if (input_char == 'H'){
                    terminalClear();
                    if (thisPlayer->inventoryContains(Items::Potion)){
                        thisPlayer->heal();
                        std::cout << "Healed successfully! Have " << thisPlayer->getHealth() << " remaining!" << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        playerTurn = turnState::enemyTurn;
                        return;
                    }
                    else if (!thisPlayer->inventoryContains(Items::Potion)){
                        std::cout << "Cannot heal! You do not have any potions!" << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        playerTurn = turnState::enemyTurn;
                        return;
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