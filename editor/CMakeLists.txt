cmake_minimum_required(VERSION 3.11)
project("draft_editor" VERSION 0.1.0 LANGUAGES CXX)

set(HEADERS
    include/draft/editor/editor_application.hpp
    include/draft/editor/game_module_watcher.hpp
    include/draft/editor/project.hpp
    include/draft/editor/panels/dockspace_panel.hpp
)

set(SOURCES
    src/draft/editor/editor_application.cpp
    src/draft/editor/game_module_watcher.cpp
    src/draft/editor/project.cpp
    src/draft/editor/panels/dockspace_panel.cpp
    src/main.cpp
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE draft::common draft::runtime draft::build_tools)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
draft_copy_openal_dll(${PROJECT_NAME})
