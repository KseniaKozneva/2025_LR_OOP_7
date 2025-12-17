#include "gtest/gtest.h"
#include "npc_types.h"
#include "visitor.h"
#include "constants.h"
#include <memory>

TEST(NPCTest, PositionDistance) {
    Position p1{0, 0};
    Position p2{3, 4};
    EXPECT_DOUBLE_EQ(p1.distance_to(p2), 5.0);
}

TEST(NPCTest, PositionBounds) {
    Position p1{50, 50};
    EXPECT_TRUE(p1.is_within_game_bounds());
    
    Position p2{-1, 50};
    EXPECT_FALSE(p2.is_within_game_bounds());
    
    // ИСПРАВЛЕНО: EDITOR_MAX_X = 500, так что (500, 50) - это ВНЕ границ!
    // Границы: [0, EDITOR_MAX_X-1] = [0, 499]
    Position p3{499, 50};  // Было 500, стало 499
    EXPECT_TRUE(p3.is_within_editor_bounds());
    EXPECT_FALSE(p3.is_within_game_bounds());
}

TEST(NPCTest, DragonCreation) {
    auto dragon = std::make_shared<Dragon>("TestDragon", 10, 20);
    EXPECT_EQ(dragon->get_type(), NpcType::DRAGON);
    EXPECT_EQ(dragon->get_name(), "TestDragon");
    EXPECT_TRUE(dragon->is_alive());
    EXPECT_EQ(dragon->get_position().x, 10);
    EXPECT_EQ(dragon->get_position().y, 20);
}

TEST(NPCTest, DragonMovement) {
    auto dragon = std::make_shared<Dragon>("TestDragon", 50, 50);
    auto config = dragon->get_movement_config();
    EXPECT_EQ(config.move_distance, DRAGON_CONFIG.move_distance);
    EXPECT_EQ(config.kill_distance, DRAGON_CONFIG.kill_distance);
    
    // Проверяем, что движение не выходит за границы
    for (int i = 0; i < 100; ++i) {
        dragon->move();
        auto pos = dragon->get_position();
        EXPECT_GE(pos.x, 0);
        EXPECT_LE(pos.x, MAP_WIDTH - 1);
        EXPECT_GE(pos.y, 0);
        EXPECT_LE(pos.y, MAP_HEIGHT - 1);
    }
}

TEST(NPCTest, KillMethod) {
    auto npc = std::make_shared<Dragon>("TestDragon", 10, 20);
    EXPECT_TRUE(npc->is_alive());
    
    npc->kill();
    EXPECT_FALSE(npc->is_alive());
    
    // Убитый NPC не должен двигаться
    Position old_pos = npc->get_position();
    npc->move();
    EXPECT_EQ(npc->get_position().x, old_pos.x);
    EXPECT_EQ(npc->get_position().y, old_pos.y);
}

TEST(NPCTest, FightVisitor) {
    auto dragon = std::make_shared<Dragon>("Dragon", 0, 0);
    auto bull = std::make_shared<Bull>("Bull", 0, 0);
    auto frog = std::make_shared<Frog>("Frog", 0, 0);
    
    // Дракон может убить быка
    auto dragonVisitor = std::make_shared<FightVisitor>(NpcType::DRAGON);
    EXPECT_TRUE(bull->accept(dragonVisitor));
    EXPECT_FALSE(frog->accept(dragonVisitor));
    EXPECT_FALSE(dragon->accept(dragonVisitor));
    
    // Бык может убить лягушку
    auto bullVisitor = std::make_shared<FightVisitor>(NpcType::BULL);
    EXPECT_TRUE(frog->accept(bullVisitor));
    EXPECT_FALSE(bull->accept(bullVisitor));
    EXPECT_FALSE(dragon->accept(bullVisitor));
    
    // Лягушка никого не убивает
    auto frogVisitor = std::make_shared<FightVisitor>(NpcType::FROG);
    EXPECT_FALSE(dragon->accept(frogVisitor));
    EXPECT_FALSE(bull->accept(frogVisitor));
    EXPECT_FALSE(frog->accept(frogVisitor));
}

TEST(NPCTest, DiceRoll) {
    auto npc = std::make_shared<Dragon>("Test", 0, 0);
    
    for (int i = 0; i < 100; ++i) {
        int roll = npc->roll_dice();
        EXPECT_GE(roll, 1);
        EXPECT_LE(roll, DICE_SIDES);
    }
}

TEST(NPCTest, SaveLoad) {
    auto dragon = std::make_shared<Dragon>("DragonTest", 42, 24);
    
    std::stringstream ss;
    dragon->save(ss);
    
    EXPECT_EQ(ss.str(), "dragon 42 24 \"DragonTest\"");
}

TEST(NPCTest, InfoString) {
    auto dragon = std::make_shared<Dragon>("FireDragon", 10, 20);
    std::string info = dragon->info();
    EXPECT_TRUE(info.find("dragon") != std::string::npos);
    EXPECT_TRUE(info.find("\"FireDragon\"") != std::string::npos);
    EXPECT_TRUE(info.find("(10, 20)") != std::string::npos);
    
    dragon->kill();
    info = dragon->info();
    EXPECT_TRUE(info.find("[DEAD]") != std::string::npos);
}