cmake_minimum_required(VERSION 3.11)
project("draft_build_tools" VERSION 0.1.0 LANGUAGES CXX)

# Placeholder
add_library(${PROJECT_NAME} INTERFACE)
add_library(draft::build_tools ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} INTERFACE draft::common)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
