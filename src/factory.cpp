#include "factory.h"
#include <stdexcept>
#include <sstream>
#include <iostream>

std::shared_ptr<INpc> NpcFactory::create_npc(NpcType type, const std::string& base_name, int x, int y) {
    if (x < config.min_x || x > config.max_x || 
        y < config.min_y || y > config.max_y) {
        throw std::out_of_range("Coordinates must be in range [" + 
                               std::to_string(config.min_x) + ", " + 
                               std::to_string(config.max_x) + "] for x and [" +
                               std::to_string(config.min_y) + ", " + 
                               std::to_string(config.max_y) + "] for y");
    }
    
    auto unique_name = name_generator.generate_unique_name(base_name);
    
    switch (type) {
        case NpcType::DRAGON: return std::make_shared<Dragon>(unique_name, x, y);
        case NpcType::FROG: return std::make_shared<Frog>(unique_name, x, y);
        case NpcType::BULL: return std::make_shared<Bull>(unique_name, x, y);
        default: return nullptr;
    }
}

std::shared_ptr<INpc> NpcFactory::create_npc_from_stream(std::istream& in) {
    std::string type_str;
    if (!(in >> type_str)) {
        return nullptr;
    }
    
    if (type_str == "dragon") return std::make_shared<Dragon>(in);
    else if (type_str == "frog") return std::make_shared<Frog>(in);
    else if (type_str == "bull") return std::make_shared<Bull>(in);
    else return nullptr;
}

std::vector<std::shared_ptr<INpc>> NpcFactory::load_from_file(const std::string& filename) {
    std::vector<std::shared_ptr<INpc>> npcs;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return npcs;
    }
    
    int count;
    if (file >> count) {
        file.ignore(); 
        for (int i = 0; i < count; ++i) {
            auto npc = create_npc_from_stream(file);
            if (npc) {
                npcs.push_back(npc);
            }
        }
    }
    
    return npcs;
}

void NpcFactory::save_to_file(const std::string& filename, const std::vector<std::shared_ptr<INpc>>& npcs) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }
    
    int alive_count = 0;
    for (const auto& npc : npcs) {
        if (npc && npc->is_alive()) {
            alive_count++;
        }
    }
    
    file << alive_count << "\n";
    
    for (const auto& npc : npcs) {
        if (npc && npc->is_alive()) {
            npc->save(file);
            file << "\n";
        }
    }
}