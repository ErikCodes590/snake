cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)

add_executable(snake
    src/main.cpp
    src/snake.cpp
)

target_include_directories(snake PRIVATE include)

target_compile_features(snake PRIVATE cxx_std_17)

find_package(SDL3 REQUIRED)

target_link_libraries(snake PRIVATE SDL3::SDL3)
