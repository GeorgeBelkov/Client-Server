cmake_minimum_required(VERSION 3.11)
project(Client)
set(CMAKE_CXX_STANDARD 20)

add_library(Client STATIC src/Client.cpp)
target_include_directories(Client PUBLIC include)

add_executable(client main.cpp)
target_link_libraries(client Client)
