
cmake_minimum_required(VERSION 3.20)

project(GameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED CONFIG)

add_executable(GameEngine
    src/main.cpp
)

target_link_libraries(GameEngine PRIVATE SDL3::SDL3)

target_compile_options(GameEngine PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
