
cmake_minimum_required(VERSION 3.20)

project(Pong LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pong src/main.cpp)

target_compile_options(pong PRIVATE -Wall -Wextra)
