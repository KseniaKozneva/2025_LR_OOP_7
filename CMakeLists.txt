cmake_minimum_required(VERSION 3.15)
project(BalagurFate3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции компиляции
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc /D_CRT_SECURE_NO_WARNINGS")
    # Для отладки в Visual Studio
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG:FULL")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Защита от in-source сборки
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build directory and run cmake from there.")
endif()

# Основная программа
add_executable(balagur_fate
    src/main.cpp
    src/battle.cpp
    src/factory.cpp
    src/game.cpp
    src/npc_types.cpp
    src/observer.cpp
    src/visitor.cpp
)

target_include_directories(balagur_fate PRIVATE include)

# Подключаем многопоточность
find_package(Threads REQUIRED)
target_link_libraries(balagur_fate PRIVATE Threads::Threads)

# Google Test - автоматическое скачивание если не найден
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    include(FetchContent)
    
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    
    # Для Windows может потребоваться установка Windows SDK
    if(WIN32)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    
    # Проверяем, существуют ли файлы тестов
    set(TEST_FILES
        test/test_npc.cpp
        test/test_factory.cpp
        test/test_game.cpp
        test/test_battle.cpp
    )
    
    # Создаем список существующих тестовых файлов
    set(EXISTING_TEST_FILES)
    foreach(test_file ${TEST_FILES})
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_file})
            list(APPEND EXISTING_TEST_FILES ${test_file})
        endif()
    endforeach()
    
    if(EXISTING_TEST_FILES)
        message(STATUS "Found test files: ${EXISTING_TEST_FILES}")
        
        # Тесты - компилируем все исходники заново для тестов
        add_executable(balagur_fate_tests
            ${EXISTING_TEST_FILES}
            src/battle.cpp
            src/factory.cpp
            src/game.cpp
            src/npc_types.cpp
            src/observer.cpp
            src/visitor.cpp
        )
        
        target_include_directories(balagur_fate_tests PRIVATE include)
        target_link_libraries(balagur_fate_tests PRIVATE 
            Threads::Threads
            GTest::gtest 
            GTest::gtest_main
        )
        
        # Для MSVC
        if(MSVC)
            target_compile_definitions(balagur_fate_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
        endif()
        
        # Добавляем тесты
        add_test(NAME BalagurFate3Tests COMMAND balagur_fate_tests)
        
        # Опционально: добавляем discover тесты для более детального вывода
        # gtest_discover_tests(balagur_fate_tests)
    else()
        message(WARNING "Test files not found. Tests will not be built.")
    endif()
endif()

# Установка выходной директории
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

# Для Visual Studio, чтобы исполняемые файлы были в build/
set_target_properties(balagur_fate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)

if(TARGET balagur_fate_tests)
    set_target_properties(balagur_fate_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
    )
endif()