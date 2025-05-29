cmake_minimum_required(VERSION 3.20)
project(example)

find_package(game_engine)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE game_engine::engine)