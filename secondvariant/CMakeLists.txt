# Минимально требуемая версия CMake
cmake_minimum_required(VERSION 3.10)

# Название проекта и используемый язык
project(MathCalculator CXX)

# Устанавливаем стандарт C++ (C++14 достаточно для используемых функций)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавляем флаги компилятора для вывода предупреждений и оптимизации
add_compile_options(-Wall -Wextra -O2)

# Создаем исполняемый файл "calc" из твоего исходного кода
add_executable(calc main.cpp)