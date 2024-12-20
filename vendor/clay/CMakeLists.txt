cmake_minimum_required(VERSION 3.11)
project(clay)

add_library(clay)
target_sources(clay PRIVATE src/clay.c)
target_include_directories(clay PUBLIC src)