cmake_minimum_required(VERSION 3.11)
project(imgui)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(imgui_git GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.5-docking)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(imgui_git)

add_library(imgui)
target_link_libraries(imgui PRIVATE glfw)
target_sources(imgui PRIVATE
    ${imgui_git_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_git_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_git_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_git_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_git_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_git_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_git_SOURCE_DIR}/imgui.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_git_SOURCE_DIR}
    ${imgui_git_SOURCE_DIR}/backends
)