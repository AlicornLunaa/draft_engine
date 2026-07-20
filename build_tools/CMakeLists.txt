cmake_minimum_required(VERSION 3.11)
project("draft_build_tools" VERSION 0.1.0 LANGUAGES CXX)

option(DRAFT_BUILD_TOOLS_BUILD_TESTS "Build the tests for draft_build_tools" ON)

set(HEADERS
    include/draft/build_tools/apak_writer.hpp
    include/draft/build_tools/asset_pipeline.hpp
    include/draft/build_tools/game_context.hpp
    include/draft/build_tools/game_module.hpp
    include/draft/build_tools/game_module_loader.hpp
    include/draft/build_tools/project_exporter.hpp
    include/draft/build_tools/project_manifest.hpp
    include/draft/build_tools/project_packer.hpp
)

set(SOURCES
    src/draft/build_tools/apak_writer.cpp
    src/draft/build_tools/asset_pipeline.cpp
    src/draft/build_tools/game_module_loader.cpp
    src/draft/build_tools/project_exporter.cpp
    src/draft/build_tools/project_manifest.cpp
    src/draft/build_tools/project_packer.cpp
)

add_library(${PROJECT_NAME} STATIC)
add_library(draft::build_tools ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# glfw: project_packer.cpp needs a hidden GL context (glfwInit/glfwWindowHint) the same way the
# CLI mains used to before that logic moved into the library - draft::runtime only links glfw
# PRIVATE, so anything calling raw GLFW functions directly still needs its own link to it.
target_link_libraries(${PROJECT_NAME} PUBLIC draft::common draft::runtime ${CMAKE_DL_LIBS} PRIVATE miniz glfw)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

# Exposed so draft_add_game_module can compile the launcher's main() straight into a Release
# game's own executable.
set(DRAFT_LAUNCHER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/launcher/main.cpp" CACHE INTERNAL "")

# Declares `target` as this project's game module, whichever shape the active config needs:
#   Debug   - a shared library, placed next to draft_launcher, which dlopens it at run time
#             (see GameModuleLoader). Reusable: the same draft_launcher works for any project.
#   Release - a self-contained executable. The launcher's own main() is compiled straight into
#             it alongside the game's own sources, so draft_register_game() (DRAFT_GAME_MODULE)
#             is just an ordinary linked function call. Every Release game builds its own.
# `sources` should be the game's own translation units only, not the launcher's.
function(draft_add_game_module target)
    set(sources ${ARGN})

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_library(${target} SHARED ${sources})
        set_target_properties(${target} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )

        # The manifest draft_launcher/the editor read to find this module.
        file(RELATIVE_PATH DRAFT_MODULE_OUTPUT_DIR_REL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/bin")
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND $<TARGET_FILE:draft_buildtools> manifest ${CMAKE_SOURCE_DIR} "${DRAFT_MODULE_OUTPUT_DIR_REL}/$<TARGET_FILE_NAME:${target}>"
            COMMENT "Updating ${CMAKE_SOURCE_DIR}/manifest.json"
        )
    else()
        add_executable(${target} ${sources} ${DRAFT_LAUNCHER_SOURCE})
        target_compile_definitions(${target} PRIVATE DRAFT_STATIC_GAME_MODULE)

        # Lets export_project() find this binary generically
        install(TARGETS ${target} RUNTIME DESTINATION . COMPONENT DraftGameLauncher)
    endif()

    target_link_libraries(${target} PRIVATE draft::build_tools draft::runtime)
endfunction()

# The generic Debug launcher (see draft_add_game_module above). Meaningless in Release, where
# every game is its own self-contained executable instead, so it's Debug-only.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(draft_launcher ${DRAFT_LAUNCHER_SOURCE})
    target_link_libraries(draft_launcher PRIVATE ${PROJECT_NAME} draft::runtime)
    target_compile_features(draft_launcher PRIVATE cxx_std_23)
    draft_copy_openal_dll(draft_launcher)
endif()

# Standalone asset-pipeline CLI
add_executable(draft_buildtools src/cli/main.cpp src/validate/main.cpp src/pack/main.cpp src/export/main.cpp src/manifest/main.cpp)
target_link_libraries(draft_buildtools PRIVATE ${PROJECT_NAME} draft::runtime glfw)
target_compile_features(draft_buildtools PRIVATE cxx_std_23)
draft_copy_openal_dll(draft_buildtools)

if(DRAFT_BUILD_TOOLS_BUILD_TESTS)
    message(STATUS "Building draft_build_tools tests")
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip SYSTEM)
    FetchContent_MakeAvailable(googletest)

    file(GLOB_RECURSE BUILD_TOOLS_TEST_SOURCES "tests/**/*.cpp")

    add_executable(
        draft_build_tools_tests
        ${BUILD_TOOLS_TEST_SOURCES}
    )

    target_link_libraries(
        draft_build_tools_tests
        PRIVATE
        GTest::gtest_main
        ${PROJECT_NAME}
        draft::common
        glfw
    )

    target_compile_features(draft_build_tools_tests PRIVATE cxx_std_23)

    include(GoogleTest)
    gtest_discover_tests(draft_build_tools_tests)
endif()
