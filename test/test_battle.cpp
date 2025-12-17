#include "gtest/gtest.h"
#include "battle.h"
#include "npc_types.h"
#include <memory>

// Мок-наблюдатель для тестирования
class MockObserver : public IObserver {
public:
    int kill_count = 0;
    std::shared_ptr<INpc> last_killer;
    std::shared_ptr<INpc> last_victim;
    
    void on_kill(const std::shared_ptr<INpc>& killer, const std::shared_ptr<INpc>& victim) override {
        kill_count++;
        last_killer = killer;
        last_victim = victim;
    }
};

TEST(BattleTest, AddRemoveObserver) {
    Battle battle;
    auto observer = std::make_shared<MockObserver>();
    
    battle.add_observer(observer);
    // Не можем проверить напрямую, но проверяем что нет исключений
    EXPECT_NO_THROW(battle.clear_observers());
}

TEST(BattleTest, FightVisitorLogic) {
    auto dragon = std::make_shared<Dragon>("Dragon", 0, 0);
    auto bull = std::make_shared<Bull>("Bull", 0, 0);
    auto frog = std::make_shared<Frog>("Frog", 0, 0);
    
    // Проверяем логику посетителя
    auto dragon_visitor = std::make_shared<FightVisitor>(NpcType::DRAGON);
    EXPECT_TRUE(dragon_visitor->visit(bull));
    EXPECT_FALSE(dragon_visitor->visit(frog));
    EXPECT_FALSE(dragon_visitor->visit(dragon));
    
    auto bull_visitor = std::make_shared<FightVisitor>(NpcType::BULL);
    EXPECT_TRUE(bull_visitor->visit(frog));
    EXPECT_FALSE(bull_visitor->visit(bull));
    EXPECT_FALSE(bull_visitor->visit(dragon));
    
    auto frog_visitor = std::make_shared<FightVisitor>(NpcType::FROG);
    EXPECT_FALSE(frog_visitor->visit(dragon));
    EXPECT_FALSE(frog_visitor->visit(bull));
    EXPECT_FALSE(frog_visitor->visit(frog));
}

TEST(BattleTest, BattleDistance) {
    std::vector<std::shared_ptr<INpc>> npcs;
    auto dragon = std::make_shared<Dragon>("Dragon", 0, 0);
    auto bull = std::make_shared<Bull>("Bull", 5, 5);  // Расстояние ~7.07
    auto bull_far = std::make_shared<Bull>("BullFar", 100, 100);  // Далеко
    
    npcs.push_back(dragon);
    npcs.push_back(bull);
    npcs.push_back(bull_far);
    
    Battle battle;
    
    // Битва с дистанцией 10 - должен убить только ближнего быка
    battle.fight(npcs, 10);
    
    EXPECT_TRUE(dragon->is_alive());
    EXPECT_FALSE(bull->is_alive());      // Убит
    EXPECT_TRUE(bull_far->is_alive());   // Жив (далеко)
    EXPECT_EQ(npcs.size(), 2);           // Удален мертвый NPC
}

