cmake_minimum_required(VERSION 3.10)
project(MathCalculator CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -O2)

# 1. Создаем библиотеку
add_library(calc_lib
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/utils.cpp
)
target_include_directories(calc_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 2. Создаем основной исполняемый файл
add_executable(calc src/main.cpp)
target_link_libraries(calc PRIVATE calc_lib)

# 3. Настройка тестирования
enable_testing()

# Создаем тестовый исполняемый файл
add_executable(unit_tests tests/test_main.cpp)
target_link_libraries(unit_tests PRIVATE calc_lib)

# Регистрируем тест в системе CTest
add_test(NAME AllTests COMMAND unit_tests)