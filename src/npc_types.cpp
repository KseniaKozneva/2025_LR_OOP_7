#include "npc_types.h"
#include <cmath>
#include <stdexcept>
#include <chrono>
#include <sstream>

BaseNpc::BaseNpc(NpcType type, const std::string& name, int x, int y) 
    : position{x, y}, name(name), type(type) {
    auto seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    rng.seed(seed);
}

Position BaseNpc::get_position() const { return position; }
std::string BaseNpc::get_name() const { return name; }
NpcType BaseNpc::get_type() const { return type; }

// ИСПРАВЛЕНО: используем atomic load
bool BaseNpc::is_alive() const { return alive.load(); }

// ИСПРАВЛЕНО: используем atomic store
void BaseNpc::kill() { 
    alive.store(false); 
}

MovementConfig BaseNpc::get_movement_config() const {
    switch(type) {
        case NpcType::DRAGON: return DRAGON_CONFIG;
        case NpcType::FROG: return FROG_CONFIG;
        case NpcType::BULL: return BULL_CONFIG;
        default: return {0, 0};
    }
}

int BaseNpc::roll_dice() const {
    std::uniform_int_distribution<int> dist(1, DICE_SIDES);
    return dist(rng);
}

std::string BaseNpc::get_type_str() const {
    switch(type) {
        case NpcType::DRAGON: return "dragon";
        case NpcType::FROG: return "frog";
        case NpcType::BULL: return "bull";
        default: return "unknown";
    }
}

std::string BaseNpc::info() const {
    return get_type_str() + " \"" + name + "\" " + position.to_string() +
           (alive.load() ? "" : " [DEAD]");
}

bool BaseNpc::accept(const std::shared_ptr<IVisitor>& visitor) {
    return visitor ? visitor->visit(shared_from_this()) : false;
}

void BaseNpc::move() {
    if (!alive.load()) return;
    specific_move();
    
    // Проверяем границы игры (100x100)
    if (!position.is_within_game_bounds()) {
        position.x = std::max(0, std::min(position.x, MAP_WIDTH - 1));
        position.y = std::max(0, std::min(position.y, MAP_HEIGHT - 1));
    }
}

void BaseNpc::save(std::ostream& os) const {
    os << get_type_str() << " " << position.x << " " << position.y << " \"" << name << "\"";
}

void BaseNpc::subscribe(const std::shared_ptr<IObserver>& observer) {
    if (observer) {
        observers.push_back(observer);
    }
}

void BaseNpc::notify_kill(const std::shared_ptr<INpc>& victim) {
    for (auto& observer : observers) {
        if (observer) {
            observer->on_kill(shared_from_this(), victim);
        }
    }
}

void BaseNpc::specific_move() {
    // Базовый класс не определяет движение
}

Dragon::Dragon(const std::string& name, int x, int y) : BaseNpc(NpcType::DRAGON, name, x, y) {}

Dragon::Dragon(std::istream& is) : BaseNpc(NpcType::DRAGON, "", 0, 0) {
    is >> position.x >> position.y;
    char quote;
    is >> std::ws >> quote;
    std::getline(is, name, '"');
}

void Dragon::specific_move() {
    position.random_move(DRAGON_CONFIG.move_distance);
}

Frog::Frog(const std::string& name, int x, int y) : BaseNpc(NpcType::FROG, name, x, y) {}

Frog::Frog(std::istream& is) : BaseNpc(NpcType::FROG, "", 0, 0) {
    is >> position.x >> position.y;
    char quote;
    is >> std::ws >> quote;
    std::getline(is, name, '"');
}

void Frog::specific_move() {
    position.random_move(FROG_CONFIG.move_distance);
}

Bull::Bull(const std::string& name, int x, int y) : BaseNpc(NpcType::BULL, name, x, y) {}

Bull::Bull(std::istream& is) : BaseNpc(NpcType::BULL, "", 0, 0) {
    is >> position.x >> position.y;
    char quote;
    is >> std::ws >> quote;
    std::getline(is, name, '"');
}

void Bull::specific_move() {
    position.random_move(BULL_CONFIG.move_distance);
}