cmake_minimum_required(VERSION 3.8)
project(entt)

add_library(entt)
target_sources(entt PRIVATE src/entt.cpp)
target_include_directories(entt PUBLIC include)