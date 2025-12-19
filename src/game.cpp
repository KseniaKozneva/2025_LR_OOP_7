#include "game.h"
#include "constants.h"
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>

using namespace std::chrono_literals;

Game::Game() : game_start_time(std::chrono::steady_clock::now()) {
    GameConfig editor_config = {0, EDITOR_MAX_X, 0, EDITOR_MAX_Y};
    factory.set_config(editor_config);
    
    console_observer = std::make_shared<ConsoleObserver>();
    file_observer = std::make_shared<FileObserver>("battle_log.txt");
}

Game::~Game() {
    reset_game();  
}

void Game::reset_game() {
    stop();
    
    {
        std::lock_guard<std::shared_mutex> lock(npcs_mutex);
        npcs.clear();
    }
    
    {
        std::lock_guard<std::mutex> lock(battle_queue_mutex);
        while (!battle_queue.empty()) {
            battle_queue.pop();
        }
    }
    
    factory.clear_names();
    game_running = false;
    game_start_time = std::chrono::steady_clock::now();
    
    if (movement_thread.joinable()) {
        movement_thread.join();
    }
    if (battle_thread.joinable()) {
        battle_thread.join();
    }
    
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "Game reset completed\n";
}

void Game::add_npc(NpcType type, const std::string& base_name, int x, int y) {
    try {
        auto npc = factory.create_npc(type, base_name, x, y);
        if (npc) {
            std::lock_guard<std::shared_mutex> lock(npcs_mutex);
            npc->subscribe(console_observer);
            npc->subscribe(file_observer);
            npcs.push_back(npc);
            
            std::lock_guard<std::mutex> lock_cout(cout_mutex);
            std::cout << "Added NPC: " << npc->info() << "\n";
        }
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Error: " << e.what() << "\n";
    }
}

void Game::load_from_file(const std::string& filename) {
    reset_game(); 
    auto loaded = factory.load_from_file(filename);
    
    std::lock_guard<std::shared_mutex> lock(npcs_mutex);
    for (auto& npc : loaded) {
        if (npc) {
            npc->subscribe(console_observer);
            npc->subscribe(file_observer);
            npcs.push_back(npc);
        }
    }
    
    std::lock_guard<std::mutex> lock_cout(cout_mutex);
    std::cout << "Loaded " << loaded.size() << " NPCs from " << filename << "\n";
}

void Game::save_to_file(const std::string& filename) {
    std::shared_lock<std::shared_mutex> lock(npcs_mutex);
    factory.save_to_file(filename, npcs);
    
    std::lock_guard<std::mutex> lock_cout(cout_mutex);
    std::cout << "Saved " << npcs.size() << " NPCs to " << filename << "\n";
}

void Game::print_npcs() {
    std::shared_lock<std::shared_mutex> lock(npcs_mutex);
    
    std::lock_guard<std::mutex> lock_cout(cout_mutex);
    std::cout << "\n=== NPC List (" << npcs.size() << ") ===\n";
    for (size_t i = 0; i < npcs.size(); ++i) {
        if (npcs[i]) {
            std::cout << i + 1 << ". " << npcs[i]->info() << "\n";
        }
    }
    std::cout << "=====================\n";
}

void Game::fight(int range) {
    std::shared_lock<std::shared_mutex> lock(npcs_mutex);
    
    std::vector<std::pair<std::shared_ptr<INpc>, std::shared_ptr<INpc>>> kills;
    
    for (size_t i = 0; i < npcs.size(); ++i) {
        auto& attacker = npcs[i];
        if (!attacker || !attacker->is_alive()) continue;
        
        for (size_t j = 0; j < npcs.size(); ++j) {
            if (i == j) continue;
            
            auto& defender = npcs[j];
            if (!defender || !defender->is_alive()) continue;
            
            double distance = attacker->get_position().distance_to(defender->get_position());
            if (distance <= range) {
                auto visitor = std::make_shared<FightVisitor>(attacker->get_type());
                if (defender->accept(visitor)) {
                    kills.push_back({attacker, defender});
                }
            }
        }
    }
    
    for (auto& [killer, victim] : kills) {
        if (victim->is_alive()) {
            victim->kill();
            killer->notify_kill(victim);
        }
    }
    
    cleanup_dead_npcs();
    
    std::lock_guard<std::mutex> lock_cout(cout_mutex);
    std::cout << "Battle finished. " << kills.size() << " fights occurred.\n";
}

void Game::initialize_game(int npc_count) {
    reset_game();  
    
    GameConfig game_config = {0, MAP_WIDTH - 1, 0, MAP_HEIGHT - 1};
    factory.set_config(game_config);
    
    std::lock_guard<std::shared_mutex> lock(npcs_mutex);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> type_dist(0, 2);
    std::uniform_int_distribution<int> coord_dist(0, MAP_WIDTH - 1);
    
    for (int i = 0; i < npc_count; ++i) {
        NpcType type = static_cast<NpcType>(type_dist(gen));
        std::string base_name;
        
        switch(type) {
            case NpcType::DRAGON: base_name = "Dragon"; break;
            case NpcType::FROG: base_name = "Frog"; break;
            case NpcType::BULL: base_name = "Bull"; break;
            default: continue;
        }
        
        int x = coord_dist(gen);
        int y = coord_dist(gen);
        
        try {
            auto npc = factory.create_npc(type, base_name, x, y);
            if (npc) {
                npc->subscribe(console_observer);
                npc->subscribe(file_observer);
                npcs.push_back(npc);
            }
        } catch (...) {}
    }
    
    std::lock_guard<std::mutex> lock_cout(cout_mutex);
    std::cout << "Game initialized with " << npcs.size() << " NPCs\n";
}

void Game::movement_worker() {
    while (game_running) {
        {
            std::shared_lock<std::shared_mutex> lock(npcs_mutex);
            
            if (!game_running) break;
            
            if (npcs.empty()) {
                std::this_thread::sleep_for(50ms);
                continue;
            }
            
            for (auto& npc : npcs) {
                if (!game_running) break;  
                if (npc && npc->is_alive()) {
                    npc->move();
                }
            }
        }
        
        if (!game_running) break;  
        
        check_collisions();
        std::this_thread::sleep_for(50ms);
    }
}

void Game::check_collisions() {
    std::shared_lock<std::shared_mutex> lock(npcs_mutex);
    
    if (!game_running || npcs.empty()) return;
    
    for (size_t i = 0; i < npcs.size(); ++i) {
        auto& attacker = npcs[i];
        if (!attacker || !attacker->is_alive()) continue;
        
        auto config = attacker->get_movement_config();
        
        for (size_t j = 0; j < npcs.size(); ++j) {
            if (i == j) continue;
            
            auto& defender = npcs[j];
            if (!defender || !defender->is_alive()) continue;
            
            double distance = attacker->get_position().distance_to(defender->get_position());
            if (distance <= config.kill_distance) {
                std::lock_guard<std::mutex> qlock(battle_queue_mutex);
                battle_queue.push({attacker, defender});
            }
        }
    }
}

void Game::battle_worker() {
    while (game_running) {
        BattleTask task;
        bool has_task = false;
        
        {
            std::lock_guard<std::mutex> lock(battle_queue_mutex);
            if (!battle_queue.empty()) {
                task = battle_queue.front();
                battle_queue.pop();
                has_task = true;
            }
        }
        
        if (!game_running) break;
        
        if (has_task && task.attacker && task.defender && 
            task.attacker->is_alive() && task.defender->is_alive()) {
            
            auto visitor = std::make_shared<FightVisitor>(task.attacker->get_type());
            if (task.defender->accept(visitor)) {
                int attack = task.attacker->roll_dice();
                int defense = task.defender->roll_dice();
                
                if (attack > defense) {
                    task.defender->kill();
                    task.attacker->notify_kill(task.defender);
                    
                    {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "BATTLE: " << task.attacker->get_name() 
                                  << " killed " << task.defender->get_name()
                                  << " (" << attack << " vs " << defense << ")\n";
                    }
                } else {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "BATTLE: " << task.attacker->get_name() 
                              << " missed " << task.defender->get_name()
                              << " (" << attack << " vs " << defense << ")\n";
                }
            }
        }
        
        cleanup_dead_npcs();
        std::this_thread::sleep_for(100ms);
    }
}

void Game::cleanup_dead_npcs() {
    std::lock_guard<std::shared_mutex> lock(npcs_mutex);
    
    auto new_end = std::remove_if(npcs.begin(), npcs.end(),
        [](const std::shared_ptr<INpc>& npc) {
            return !npc || !npc->is_alive();
        });
    
    if (new_end != npcs.end()) {
        npcs.erase(new_end, npcs.end());
    }
}

void Game::start() {
    if (game_running) return;
    
    game_running = true;
    game_start_time = std::chrono::steady_clock::now();
    
    movement_thread = std::thread(&Game::movement_worker, this);
    battle_thread = std::thread(&Game::battle_worker, this);
    
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "Game started!\n";
}

void Game::stop() {
    if (!game_running) return;
    
    game_running = false;
    
    std::this_thread::sleep_for(100ms);
    
    if (movement_thread.joinable()) movement_thread.join();
    if (battle_thread.joinable()) battle_thread.join();
    
    {
        std::lock_guard<std::mutex> lock(battle_queue_mutex);
        while (!battle_queue.empty()) {
            battle_queue.pop();
        }
    }
    
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "Game stopped\n";
}

int Game::get_game_time() const {
    if (!game_running) {
        auto elapsed = std::chrono::steady_clock::now() - game_start_time;
        return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
    }
    auto elapsed = std::chrono::steady_clock::now() - game_start_time;
    return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
}

void Game::print_map() {
    std::lock_guard<std::mutex> cout_lock(cout_mutex);
    std::shared_lock<std::shared_mutex> npc_lock(npcs_mutex);
    
    auto now = std::chrono::steady_clock::now();
    int game_time = static_cast<int>(
        std::chrono::duration_cast<std::chrono::seconds>(now - game_start_time).count()
    );

    char map[MAP_HEIGHT][MAP_WIDTH];
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            map[y][x] = '.';  // '.' = пустая клетка
        }
    }

    for (const auto& npc : npcs) {
        if (npc && npc->is_alive()) {
            Position pos = npc->get_position();
            if (pos.x >= 0 && pos.x < MAP_WIDTH && pos.y >= 0 && pos.y < MAP_HEIGHT) {
                if (map[pos.y][pos.x] == '.') {
                    switch (npc->get_type()) {
                        case NpcType::DRAGON: map[pos.y][pos.x] = 'D'; break;
                        case NpcType::FROG:   map[pos.y][pos.x] = 'F'; break;
                        case NpcType::BULL:   map[pos.y][pos.x] = 'B'; break;
                        default:              map[pos.y][pos.x] = '?';
                    }
                }
            }
        }
    }

    std::cout << "\n";
    std::cout << "+==================================================+\n";
    std::cout << "|                 MAP (" << std::setw(2) << game_time << "s)                   |\n";
    std::cout << "+==================================================+\n";
    std::cout << "| SYMBOLS: D=Dragon, F=Frog, B=Bull, .=Empty      |\n";
    std::cout << "+--------------------------------------------------+\n";

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            std::cout << map[y][x];
        }
        std::cout << "\n";
    }

    int alive_count = get_alive_count();
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Alive: " << std::setw(3) << alive_count
              << " | Dead: " << std::setw(3) << (npcs.size() - alive_count)
              << " | Total: " << std::setw(3) << npcs.size() << " |\n";
    std::cout << "+==================================================+\n";
}

void Game::print_survivors() {
    std::lock_guard<std::mutex> cout_lock(cout_mutex);
    std::shared_lock<std::shared_mutex> npc_lock(npcs_mutex);
    
    std::cout << "\n=== SURVIVORS ===\n";
    int count = 0;
    for (const auto& npc : npcs) {
        if (npc && npc->is_alive()) {
            std::cout << npc->info() << "\n";
            count++;
        }
    }
    
    if (count == 0) {
        std::cout << "No survivors!\n";
    } else {
        std::cout << "Total survivors: " << count << "\n";
    }
}

int Game::get_alive_count() const {
    std::shared_lock<std::shared_mutex> lock(npcs_mutex);
    int count = 0;
    for (const auto& npc : npcs) {
        if (npc && npc->is_alive()) count++;
    }
    return count;
}