cmake_minimum_required(VERSION 3.8)
project(stb)

add_library(stb)
target_sources(stb PRIVATE
    src/stb_vorbis.c
)
target_include_directories(stb PUBLIC include)