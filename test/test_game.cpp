#include "gtest/gtest.h"
#include "game.h"
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class GameTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::this_thread::sleep_for(50ms);
    }
    
    void TearDown() override {
        std::this_thread::sleep_for(50ms);
    }
};

TEST_F(GameTest, AddNPC) {
    Game game;
    game.add_npc(NpcType::DRAGON, "TestDragon", 10, 20);
    EXPECT_NO_THROW(game.print_npcs());
}

TEST_F(GameTest, InitializeGame) {
    Game game;
    game.initialize_game(10);
    
    EXPECT_GE(game.get_alive_count(), 0);
    EXPECT_LE(game.get_alive_count(), 10);
}

TEST_F(GameTest, GameTime) {
    Game game;
    int time_before = game.get_game_time();
    EXPECT_GE(time_before, 0);
    
    game.start();
    std::this_thread::sleep_for(100ms);
    game.stop();
    
    int time_after = game.get_game_time();
    EXPECT_GE(time_after, time_before);
}

TEST_F(GameTest, ResetGame) {
    Game game;
    
    game.initialize_game(5);
    int count_before = game.get_alive_count();
    EXPECT_EQ(count_before, 5);
    
    game.initialize_game(3);
    int count_after = game.get_alive_count();
    EXPECT_EQ(count_after, 3);
}



TEST_F(GameTest, SaveLoadFile) {
    Game game;
    const std::string test_filename = "test_game_save.txt";
    
    game.add_npc(NpcType::DRAGON, "Dragon1", 10, 20);
    game.add_npc(NpcType::FROG, "Frog1", 30, 40);
    
    EXPECT_NO_THROW(game.save_to_file(test_filename));
    
    Game game2;
    EXPECT_NO_THROW(game2.load_from_file(test_filename));
    
    EXPECT_GE(game2.get_alive_count(), 0);
    
    std::remove(test_filename.c_str());
}

TEST_F(GameTest, StartStop) {
    Game game;
    
    EXPECT_NO_THROW(game.start());
    std::this_thread::sleep_for(50ms);
    EXPECT_NO_THROW(game.stop());
    
    EXPECT_NO_THROW(game.start());
    std::this_thread::sleep_for(50ms);
    EXPECT_NO_THROW(game.stop());
}

TEST_F(GameTest, PrintFunctions) {
    Game game;
    
    game.initialize_game(3);
    
    EXPECT_NO_THROW(game.print_npcs());
    EXPECT_NO_THROW(game.print_map());
    EXPECT_NO_THROW(game.print_survivors());
}