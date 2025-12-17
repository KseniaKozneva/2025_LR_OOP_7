#include "gtest/gtest.h"
#include "factory.h"
#include <sstream>
#include <fstream>

TEST(FactoryTest, CreateNPC) {
    NpcFactory factory;
    
    // Создание дракона
    auto dragon = factory.create_npc(NpcType::DRAGON, "Dragon", 10, 20);
    ASSERT_NE(dragon, nullptr);
    EXPECT_EQ(dragon->get_type(), NpcType::DRAGON);
    EXPECT_EQ(dragon->get_position().x, 10);
    EXPECT_EQ(dragon->get_position().y, 20);
    
    // Создание лягушки
    auto frog = factory.create_npc(NpcType::FROG, "Frog", 30, 40);
    ASSERT_NE(frog, nullptr);
    EXPECT_EQ(frog->get_type(), NpcType::FROG);
    
    // Создание быка
    auto bull = factory.create_npc(NpcType::BULL, "Bull", 50, 60);
    ASSERT_NE(bull, nullptr);
    EXPECT_EQ(bull->get_type(), NpcType::BULL);
}

TEST(FactoryTest, UniqueNames) {
    NpcFactory factory;
    
    auto npc1 = factory.create_npc(NpcType::DRAGON, "Test", 10, 20);
    auto npc2 = factory.create_npc(NpcType::DRAGON, "Test", 30, 40);
    auto npc3 = factory.create_npc(NpcType::DRAGON, "Test", 50, 60);
    
    EXPECT_EQ(npc1->get_name(), "Test");
    EXPECT_EQ(npc2->get_name(), "Test1");
    EXPECT_EQ(npc3->get_name(), "Test2");
}

TEST(FactoryTest, ClearNames) {
    NpcFactory factory;
    
    auto npc1 = factory.create_npc(NpcType::DRAGON, "Test", 10, 20);
    EXPECT_EQ(npc1->get_name(), "Test");
    
    factory.clear_names();
    
    auto npc2 = factory.create_npc(NpcType::DRAGON, "Test", 30, 40);
    EXPECT_EQ(npc2->get_name(), "Test");  // Снова может быть "Test"
}

TEST(FactoryTest, CreateFromStream) {
    NpcFactory factory;
    
    std::stringstream ss_dragon("dragon 42 24 \"FireDragon\"");
    auto dragon = factory.create_npc_from_stream(ss_dragon);
    ASSERT_NE(dragon, nullptr);
    EXPECT_EQ(dragon->get_type(), NpcType::DRAGON);
    EXPECT_EQ(dragon->get_name(), "FireDragon");
    EXPECT_EQ(dragon->get_position().x, 42);
    EXPECT_EQ(dragon->get_position().y, 24);
    
    std::stringstream ss_frog("frog 10 20 \"JumpFrog\"");
    auto frog = factory.create_npc_from_stream(ss_frog);
    ASSERT_NE(frog, nullptr);
    EXPECT_EQ(frog->get_type(), NpcType::FROG);
    
    std::stringstream ss_bull("bull 30 40 \"AngryBull\"");
    auto bull = factory.create_npc_from_stream(ss_bull);
    ASSERT_NE(bull, nullptr);
    EXPECT_EQ(bull->get_type(), NpcType::BULL);
}

TEST(FactoryTest, InvalidCoordinates) {
    NpcFactory factory;
    GameConfig config{0, 100, 0, 100};
    factory.set_config(config);
    
    // Координаты вне диапазона должны выбрасывать исключение
    EXPECT_THROW(factory.create_npc(NpcType::DRAGON, "Test", -10, 50), std::out_of_range);
    EXPECT_THROW(factory.create_npc(NpcType::DRAGON, "Test", 150, 50), std::out_of_range);
    EXPECT_THROW(factory.create_npc(NpcType::DRAGON, "Test", 50, -10), std::out_of_range);
    EXPECT_THROW(factory.create_npc(NpcType::DRAGON, "Test", 50, 150), std::out_of_range);
}

TEST(FactoryTest, SaveToFile) {
    NpcFactory factory;
    std::vector<std::shared_ptr<INpc>> npcs;
    
    npcs.push_back(factory.create_npc(NpcType::DRAGON, "Dragon1", 10, 20));
    npcs.push_back(factory.create_npc(NpcType::FROG, "Frog1", 30, 40));
    npcs.push_back(factory.create_npc(NpcType::BULL, "Bull1", 50, 60));
    
    // Убиваем одного NPC
    npcs[1]->kill();
    
    const std::string test_filename = "test_save.txt";
    factory.save_to_file(test_filename, npcs);
    
    // Проверяем содержимое файла
    std::ifstream file(test_filename);
    ASSERT_TRUE(file.is_open());
    
    int count;
    file >> count;
    EXPECT_EQ(count, 2);  // Только 2 живых NPC
    
    std::string line;
    std::getline(file, line);  // Пропускаем оставшуюся часть строки
    
    int lines_read = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) lines_read++;
    }
    
    EXPECT_EQ(lines_read, 2);  // Две строки с NPC
    
    file.close();
    std::remove(test_filename.c_str());
}