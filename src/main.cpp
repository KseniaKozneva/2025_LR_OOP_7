#include "../include/game.h"
#include "../include/constants.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <iomanip>

using namespace std::chrono_literals;

void print_help() {
    std::cout << "\n";
    std::cout << "+========================================+\n";
    std::cout << "|        BALAGUR FATE 3                |\n";
    std::cout << "+========================================+\n";
    std::cout << "|           EDITOR MODE                |\n";
    std::cout << "+----------------------------------------+\n";
    std::cout << "| 1 - Add NPC                          |\n";
    std::cout << "| 2 - List NPCs                        |\n";
    std::cout << "| 3 - Save to file                     |\n";
    std::cout << "| 4 - Load from file                   |\n";
    std::cout << "| 5 - Start battle (editor mode)       |\n";
    std::cout << "+----------------------------------------+\n";
    std::cout << "|           GAME MODE (Lab 7)          |\n";
    std::cout << "+----------------------------------------+\n";
    std::cout << "| 6 - Initialize game (50 NPCs)        |\n";
    std::cout << "| 7 - Start auto-battle (30 seconds)   |\n";
    std::cout << "| 8 - Print map                        |\n";
    std::cout << "| 9 - Print survivors                  |\n";
    std::cout << "| 0 - Exit                             |\n";
    std::cout << "| h - Help                             |\n";
    std::cout << "+========================================+\n";
}

NpcType select_npc_type() {
    int choice;
    while (true) {
        std::cout << "\nSelect NPC type:\n";
        std::cout << "+-----------------+\n";
        std::cout << "| 1. Dragon       |\n";
        std::cout << "| 2. Frog         |\n";
        std::cout << "| 3. Bull         |\n";
        std::cout << "+-----------------+\n";
        std::cout << "Choice: ";
        std::cin >> choice;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input\n";
            continue;
        }
        
        switch (choice) {
            case 1: return NpcType::DRAGON;
            case 2: return NpcType::FROG;
            case 3: return NpcType::BULL;
            default: std::cout << "Invalid choice\n";
        }
    }
}

int main() {
    Game game;
    std::string filename = "dungeon.txt";
    
    std::cout << "+==============================================================+\n";
    std::cout << "|        BALAGUR FATE 3 - DUNGEON EDITOR & BATTLE            |\n";
    std::cout << "+==============================================================+\n";
    print_help();
    
    while (true) {
        std::cout << "\nCommand: ";
        char command;
        std::cin >> command;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        
        try {
            switch (command) {
                case '1': {
                    auto type = select_npc_type();
                    std::string base_name;
                    std::cout << "Enter base name: ";
                    std::cin >> base_name;
                    int x, y;
                    std::cout << "Enter X coordinate [0-500]: ";
                    std::cin >> x;
                    std::cout << "Enter Y coordinate [0-500]: ";
                    std::cin >> y;
                    game.add_npc(type, base_name, x, y);
                    break;
                }
                case '2':
                    game.print_npcs();
                    break;
                case '3':
                    game.save_to_file(filename);
                    break;
                case '4':
                    game.load_from_file(filename);
                    break;
                case '5': {
                    int range;
                    std::cout << "Enter battle range: ";
                    std::cin >> range;
                    game.fight(range);
                    break;
                }
                case '6':
                    // stop уже вызывается внутри initialize_game через reset_game
                    game.initialize_game(INITIAL_NPC_COUNT);
                    break;
                case '7': {
                    std::cout << "\n";
                    std::cout << "+========================================+\n";
                    std::cout << "|      STARTING AUTO-BATTLE             |\n";
                    std::cout << "|          " << std::setw(2) << GAME_DURATION_SECONDS << " SECONDS                  |\n";
                    std::cout << "+========================================+\n\n";
                    
                    game.start();
                    
                    // Просто ждём 30 секунд, НЕ вызывая print_map
                    for (int i = 0; i < GAME_DURATION_SECONDS; ++i) {
                        std::this_thread::sleep_for(1s);
                        std::cout << "." << std::flush;
                    }
                    
                    game.stop();
                    
                    std::cout << "\n\n";
                    std::cout << "+========================================+\n";
                    std::cout << "|           GAME OVER                    |\n";
                    std::cout << "|           FINAL RESULTS                |\n";
                    std::cout << "+========================================+\n";
                    game.print_survivors();
                    break;
                }
                case '8':
                    game.print_map();
                    break;
                case '9':
                    game.print_survivors();
                    break;
                case '0':
                    game.stop();
                    std::cout << "\nGoodbye!\n";
                    return 0;
                case 'h':
                    print_help();
                    break;
                default:
                    std::cout << "Unknown command\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
        
        std::cin.ignore(10000, '\n');
    }
    
    return 0;
}