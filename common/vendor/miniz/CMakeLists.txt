cmake_minimum_required(VERSION 3.11)
project(miniz)

add_library(miniz)
target_sources(miniz PRIVATE src/miniz.c)
target_include_directories(miniz PUBLIC include)
