#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "minigame/campaign.hpp"
#include "minigame/combat.hpp"
#include "minigame/random.hpp"
#include "minigame/save_game.hpp"
#include "minigame/terminal_ui.hpp"

namespace {
using minigame::BattleState;
using minigame::Campaign;
using minigame::CombatEngine;
using minigame::ExploreType;
using minigame::PlayerAction;
using minigame::Random;
using minigame::SaveGame;
using minigame::TerminalUI;

constexpr const char* kSaveFile = "minigame.save";

std::optional<PlayerAction> actionFor(char command) {
    switch (command) {
        case 'A': return PlayerAction::Attack;
        case 'D': return PlayerAction::Defend;
        case 'H': return PlayerAction::Heal;
        case 'B': return PlayerAction::Bomb;
        case 'R': return PlayerAction::Run;
        default: return std::nullopt;
    }
}

bool loadCampaign(Campaign& campaign, TerminalUI& ui) {
    std::string error;
    const auto loaded = SaveGame::load(kSaveFile, error);
    if (!loaded) {
        ui.message("Load failed: " + error);
        return false;
    }
    campaign.restore(*loaded);
    ui.message("Campaign loaded.");
    return true;
}

void saveCampaign(const Campaign& campaign, TerminalUI& ui) {
    std::string error;
    if (SaveGame::save(campaign, kSaveFile, error)) {
        ui.message("Campaign saved to minigame.save.");
    } else {
        ui.message("Save failed: " + error);
    }
}

BattleState runBattle(Campaign& campaign, minigame::Enemy enemy, Random& random, TerminalUI& ui) {
    CombatEngine combat(campaign.player(), std::move(enemy), random);
    std::vector<std::string> events;

    while (combat.state() == BattleState::Ongoing) {
        ui.clear();
        ui.battle(campaign, combat.enemy(), events);
        const auto action = actionFor(ui.command("\nAction: "));
        if (!action) {
            events = {"Unknown action. Choose A, D, H, B, or R."};
            continue;
        }
        auto result = combat.perform(*action);
        events = std::move(result.events);
    }

    ui.clear();
    ui.battle(campaign, combat.enemy(), events);
    if (combat.state() == BatteState::Victory) {
        campaign.resolveVictory(combat.enemy());
        ui.message("\nVictory rewards claimed. Progression and boss milestones updated.");
    } else if (combat.state() == BattleState::Escaped) {
        ui.message("\nYou escaped with your current inventory intact.");
    } else {
        ui.message("\nYour run has ended. Load an earlier save or start a new campaign.");
    }
    ui.pause();
    return combat.state();
}

Campaign newCampaign(TerminalUI& ui) {
    ui.clear();
    ui.banner();
    std::string name = ui.prompt("Name your adventurer: ");
    Campaign campaign(name.empty() ? "Adventurer" : name);
    campaign.player().addItem(minigame::Item::Potion, 2);
    campaign.player().addItem(minigame::Item::Bomb, 1);
    return campaign;
}
}  // namespace

int main() {
    TerminalUI ui;
    Random random;

    ui.clear();
    ui.banner();
    const char start = ui.command("[N] New campaign   [L] Load campaign   [Q] Quit\n\nChoice: ");
    if (start == 'Q') {
        return 0;
    }

    Campaign campaign("Adventurer");
    if (start == 'L') {
        if (!loadCampaign(campaign, ui)) {
            ui.pause();
            campaign = newCampaign(ui);
        }
    } else {
        campaign = newCampaign(ui);
    }

    bool running = true;
    while (running && campaign.player().alive()) {
        ui.clear();
        ui.banner();
        ui.world(campaign);

        switch (ui.command("\nChoice: ")) {
            case 'E': {
                const auto outcome = campaign.explore(random);
                ui.clear();
                ui.message(outcome.message);

                if ((outcome.type == ExploreType::Battle || outcome.type == ExploreType::Boss) &&
                    outcome.enemy) {
                    ui.pause();
                    if (runBattle(campaign, *outcome.enemy, random, ui) == BattleState::Defeat) {
                        running = false;
                    }
                } else if (outcome.type == ExploreType::Loot && outcome.item) {
                    ui.message("Found: " + std::string(minigame::toString(*outcome.item)) +
                              " and " + std::to_string(outcome.goldFound) + " gold.");
                    ui.pause();
                } else {
                    ui.pause();
                }
                break;
            }
            case 'I':
                ui.clear();
                ui.inventory(campaign);
                ui.pause();
                break;
            case 'S':
                saveCampaign(campaign, ui);
                ui.pause();
                break;
            case 'L':
                loadCampaign(campaign, ui);
                ui.pause();
                break;
            case 'Q':
                running = false;
                break;
            default:
                ui.message("Unknown command.");
                ui.pause();
                break;
        }
    }

    ui.clear();
    if (!campaign.player().alive()) {
        ui.message("Game over.");
    }
    ui.message("Thanks for playing MiniGame.");
    return 0;
}
