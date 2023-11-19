cmake_minimum_required(VERSION 3.11)
project(Server)
set(CMAKE_CXX_STANDARD 20)

add_library(Server STATIC src/Server.cpp src/Player.cpp)
target_include_directories(Server PUBLIC include)

add_executable(server main.cpp)
target_link_libraries(server Server)