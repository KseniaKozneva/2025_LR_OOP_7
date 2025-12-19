#pragma once

#include "npc.h"
#include "factory.h"
#include "observer.h"
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <atomic>
#include <chrono>

struct BattleTask {
    std::shared_ptr<INpc> attacker;
    std::shared_ptr<INpc> defender;
};

class Game {
private:
    std::vector<std::shared_ptr<INpc>> npcs;
    mutable std::shared_mutex npcs_mutex;
    
    std::queue<BattleTask> battle_queue;
    std::mutex battle_queue_mutex;
    
    NpcFactory factory;
    std::shared_ptr<ConsoleObserver> console_observer;
    std::shared_ptr<FileObserver> file_observer;
    
    std::chrono::steady_clock::time_point game_start_time;
    std::atomic<bool> game_running{false};
    
    std::thread movement_thread;
    std::thread battle_thread;
    
    mutable std::mutex cout_mutex;
    
    void movement_worker();
    void battle_worker();
    void check_collisions();
    void cleanup_dead_npcs();
    
public:
    Game();
    ~Game();
    void add_npc(NpcType type, const std::string& base_name, int x, int y);
    void load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename);
    void print_npcs();
    void fight(int range);

    void initialize_game(int npc_count);
    void reset_game();  
    void start();
    void stop();
    
    void print_map();
    void print_survivors();
    int get_alive_count() const;
    int get_game_time() const;
    
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
};