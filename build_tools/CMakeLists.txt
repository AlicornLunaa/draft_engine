cmake_minimum_required(VERSION 3.11)
project("draft_build_tools" VERSION 0.1.0 LANGUAGES CXX)

set(HEADERS
    include/draft/build_tools/game_context.hpp
    include/draft/build_tools/game_module.hpp
    include/draft/build_tools/game_module_loader.hpp
)

set(SOURCES
    src/draft/build_tools/game_module_loader.cpp
)

add_library(${PROJECT_NAME} STATIC)
add_library(draft::build_tools ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC draft::common draft::runtime ${CMAKE_DL_LIBS})
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

        # The manifest draft_launcher reads to find this module. Generated at build time so the
        # module filename always matches whatever this platform actually names the shared library.
        file(GENERATE
            OUTPUT "$<TARGET_FILE_DIR:${target}>/game.json"
            CONTENT "{\n  \"module\": \"$<TARGET_FILE_NAME:${target}>\"\n}\n"
        )
    else()
        add_executable(${target} ${sources} ${DRAFT_LAUNCHER_SOURCE})
        target_compile_definitions(${target} PRIVATE DRAFT_STATIC_GAME_MODULE)
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
