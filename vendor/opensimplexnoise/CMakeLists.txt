cmake_minimum_required(VERSION 3.11)
project(simplex_noise)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

add_library(simplex_noise)
target_sources(simplex_noise PRIVATE
    src/OpenSimplexNoise.cpp
)
target_link_libraries(imgui PRIVATE)
target_include_directories(imgui PUBLIC include)