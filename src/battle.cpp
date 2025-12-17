#include "battle.h"
#include <algorithm>

void Battle::add_observer(std::shared_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock(mutex);
    observers.push_back(observer);
}

void Battle::remove_observer(std::shared_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock(mutex);
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Battle::clear_observers() {
    std::lock_guard<std::mutex> lock(mutex);
    observers.clear();
}

void Battle::notify_observers(const std::shared_ptr<INpc>& killer, const std::shared_ptr<INpc>& victim) {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& observer : observers) {
        if (observer) {
            observer->on_kill(killer, victim);
        }
    }
}

void Battle::fight(std::vector<std::shared_ptr<INpc>>& npcs, int range) {
    std::vector<std::pair<std::shared_ptr<INpc>, std::shared_ptr<INpc>>> kills;

    for (size_t i = 0; i < npcs.size(); ++i) {
        auto& attacker = npcs[i];
        if (!attacker || !attacker->is_alive()) continue;

        for (size_t j = 0; j < npcs.size(); ++j) {
            if (i == j) continue;

            auto& defender = npcs[j];
            if (!defender || !defender->is_alive()) continue;

            double distance = attacker->get_position().distance_to(defender->get_position());
            if (distance <= static_cast<double>(range)) {
                auto visitor = std::make_shared<FightVisitor>(attacker->get_type());
                if (defender->accept(visitor)) {
                    kills.push_back({attacker, defender});
                }
            }
        }
    }

    for (auto& [killer, victim] : kills) {
        if (victim && victim->is_alive()) {
            victim->kill();
            notify_observers(killer, victim);
        }
    }

    auto new_end = std::remove_if(npcs.begin(), npcs.end(),
        [](const std::shared_ptr<INpc>& npc) {
            return !npc || !npc->is_alive();
        });
    npcs.erase(new_end, npcs.end());
}