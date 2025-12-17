#pragma once

#include <string>
#include <memory>
#include <cmath>
#include <random>
#include <sstream>
#include "constants.h"

class IVisitor;
class IObserver;

enum class NpcType { DRAGON, FROG, BULL };

struct Position {
    int x, y;

    double distance_to(const Position& other) const {
        return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
    }

    void random_move(int max_distance) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(-max_distance, max_distance);
        x += dist(gen);
        y += dist(gen);
    }

    bool is_within_editor_bounds() const {
        return x >= 0 && x < EDITOR_MAX_X && y >= 0 && y < EDITOR_MAX_Y;
    }

    bool is_within_game_bounds() const {
        return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
    }

    std::string to_string() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

class INpc {
public:
    virtual ~INpc() = default;
    virtual Position get_position() const = 0;
    virtual std::string get_name() const = 0;
    virtual NpcType get_type() const = 0;
    virtual std::string get_type_str() const = 0;
    virtual std::string info() const = 0;
    virtual bool is_alive() const = 0;
    virtual void kill() = 0;
    virtual bool accept(const std::shared_ptr<IVisitor>& visitor) = 0;
    virtual void move() = 0;
    virtual MovementConfig get_movement_config() const = 0;
    virtual int roll_dice() const = 0;
    virtual void save(std::ostream& os) const = 0;
    virtual void subscribe(const std::shared_ptr<IObserver>& observer) = 0;
    virtual void notify_kill(const std::shared_ptr<INpc>& victim) = 0;
};