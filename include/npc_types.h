#pragma once

#include "npc.h"
#include "visitor.h"
#include "observer.h"
#include <memory>
#include <random>
#include <vector>
#include <ostream>
#include <atomic>  

class BaseNpc : public INpc, public std::enable_shared_from_this<BaseNpc> {
protected:
    Position position;
    std::string name;
    NpcType type;
    std::atomic<bool> alive{true}; 
    mutable std::mt19937 rng;
    std::vector<std::shared_ptr<IObserver>> observers;
    
public:
    BaseNpc(NpcType type, const std::string& name, int x, int y);
    Position get_position() const override;
    std::string get_name() const override;
    NpcType get_type() const override;
    std::string get_type_str() const override;
    std::string info() const override;
    bool is_alive() const override;
    void kill() override;
    bool accept(const std::shared_ptr<IVisitor>& visitor) override;
    void move() override;
    MovementConfig get_movement_config() const override;
    int roll_dice() const override;
    void save(std::ostream& os) const override;
    void subscribe(const std::shared_ptr<IObserver>& observer) override;
    void notify_kill(const std::shared_ptr<INpc>& victim) override;
    
protected:
    virtual void specific_move() = 0;
};

class Dragon : public BaseNpc {
public:
    Dragon(const std::string& name, int x, int y);
    Dragon(std::istream& is);
protected:
    void specific_move() override;
};

class Frog : public BaseNpc {
public:
    Frog(const std::string& name, int x, int y);
    Frog(std::istream& is);
protected:
    void specific_move() override;
};

class Bull : public BaseNpc {
public:
    Bull(const std::string& name, int x, int y);
    Bull(std::istream& is);
protected:
    void specific_move() override;
};