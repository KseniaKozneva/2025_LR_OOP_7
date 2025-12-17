#pragma once
#include "npc_types.h"
#include <memory>
#include <fstream>
#include <vector>
#include <set>

struct GameConfig {
    int min_x = 0;
    int max_x = EDITOR_MAX_X;
    int min_y = 0;
    int max_y = EDITOR_MAX_Y;
};

class NameGenerator {
private:
    std::set<std::string> used_names;
public:
    std::string generate_unique_name(const std::string& base_name) {
        std::string name = base_name;
        int counter = 1;
        while (used_names.find(name) != used_names.end()) {
            name = base_name + std::to_string(counter++);
        }
        used_names.insert(name);
        return name;
    }
    void clear() { used_names.clear(); }
};

class NpcFactory {
private:
    NameGenerator name_generator;
    GameConfig config;
    
public:
    NpcFactory() = default;
    NpcFactory(const GameConfig& config) : config(config) {}
    
    void set_config(const GameConfig& new_config) { config = new_config; }
    GameConfig get_config() const { return config; }
    void clear_names() { name_generator.clear(); }
    
    std::shared_ptr<INpc> create_npc(NpcType type, const std::string& base_name, int x, int y);
    std::shared_ptr<INpc> create_npc_from_stream(std::istream& in);
    std::vector<std::shared_ptr<INpc>> load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename, const std::vector<std::shared_ptr<INpc>>& npcs);
};