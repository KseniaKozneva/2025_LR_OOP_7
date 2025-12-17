#pragma once

#include <memory>
#include "npc.h"

class IVisitor {
public:
    virtual ~IVisitor() = default;
    virtual bool visit(const std::shared_ptr<INpc>& npc) = 0;
};

class FightVisitor : public IVisitor {
private:
    NpcType attacker_type;
public:
    FightVisitor(NpcType attacker_type) : attacker_type(attacker_type) {}
    bool visit(const std::shared_ptr<INpc>& npc) override;
};