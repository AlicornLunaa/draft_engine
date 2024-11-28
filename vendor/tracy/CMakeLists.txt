cmake_minimum_required(VERSION 3.11)
project(tracy)

add_library(tracy)
target_sources(tracy PRIVATE
    src/TracyClient.cpp
)
target_compile_definitions(tracy PUBLIC $<$<CONFIG:Debug>:TRACY_ENABLE>)
target_include_directories(tracy PUBLIC src)