#pragma once

#include "npc.h"
#include "visitor.h"
#include "observer.h"
#include <vector>
#include <memory>
#include <mutex>

class Battle {
private:
    std::vector<std::shared_ptr<IObserver>> observers;
    mutable std::mutex mutex;

public:
    void add_observer(std::shared_ptr<IObserver> observer);
    void remove_observer(std::shared_ptr<IObserver> observer);
    void clear_observers();
    void fight(std::vector<std::shared_ptr<INpc>>& npcs, int range);
    void notify_observers(const std::shared_ptr<INpc>& killer, const std::shared_ptr<INpc>& victim);
};