# Require CMake version 3.20 or newer
cmake_minimum_required(VERSION 3.20)

# Create a project named Pong that uses C++
project(Pong LANGUAGES CXX)

# Request the C++20 language standard
set(CMAKE_CXX_STANDARD 20)

# Do not fall back to an older C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the installed SDL3 library (required)
find_package(SDL3 REQUIRED CONFIG)

# Create an executable named pong from main.cpp
add_executable(pong src/main.cpp)

# Enable extra compiler warnings for pong
target_compile_options(pong PRIVATE -Wall -Wextra)

# Link the pong executable against SDL3
target_link_libraries(pong PRIVATE SDL3::SDL3)