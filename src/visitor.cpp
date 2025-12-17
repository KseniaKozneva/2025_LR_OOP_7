#include "visitor.h"
#include "npc.h"
#include <iostream>

bool FightVisitor::visit(const std::shared_ptr<INpc>& npc) {
    if (!npc || !npc->is_alive()) return false;

    // Дракон убивает только быков
    if (attacker_type == NpcType::DRAGON) {
        return npc->get_type() == NpcType::BULL;
    }
    // Бык убивает лягушек
    else if (attacker_type == NpcType::BULL) {
        return npc->get_type() == NpcType::FROG;
    }
    // Лягушка никого не убивает
    else {
        return false;
    }
}