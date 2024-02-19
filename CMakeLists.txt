cmake_minimum_required(VERSION 3.8)
project("space_game" VERSION 0.0.1 LANGUAGES CXX)

set(SOURCES
    src/clydesdale/core/application.cpp
    src/clydesdale/core/scene.cpp
    src/clydesdale/core/entity.cpp
    src/clydesdale/interface/element.cpp
    src/clydesdale/interface/panel.cpp
    src/clydesdale/interface/button.cpp
    src/clydesdale/math/transform.cpp
    src/clydesdale/util/logger.cpp
    src/clydesdale/util/asset_manager.cpp
    src/clydesdale/widgets/console.cpp

    src/space_game/core/game.cpp
    src/space_game/scenes/test_scene.cpp
    src/space_game/simulation/world.cpp
    
    src/main.cpp
)

set(HEADERS
    src/clydesdale/components/control_component.hpp
    src/clydesdale/components/empty_component.hpp
    src/clydesdale/components/sprite_component.hpp
    src/clydesdale/components/transform_component.hpp
    src/clydesdale/core/application.hpp
    src/clydesdale/core/scene.hpp
    src/clydesdale/core/entity.hpp
    src/clydesdale/interface/element.hpp
    src/clydesdale/interface/panel.hpp
    src/clydesdale/interface/button.hpp
    src/clydesdale/math/transform.hpp
    src/clydesdale/math/vector2.hpp
    src/clydesdale/math/vector3.hpp
    src/clydesdale/phys/world.hpp
    src/clydesdale/phys/rigid_body.hpp
    src/clydesdale/phys/collider.hpp
    src/clydesdale/phys/shape.hpp
    src/clydesdale/util/ansi_colors.hpp
    src/clydesdale/util/asset_manager.hpp
    src/clydesdale/util/constants.hpp
    src/clydesdale/util/logger.hpp
    src/clydesdale/widgets/console.hpp
    src/clydesdale/engine.hpp

    src/space_game/core/game.hpp
    src/space_game/scenes/test_scene.hpp
    src/space_game/simulation/world.hpp
    src/space_game/simulation/cell.hpp
)

set(SFML_STATIC_LIBRARIES TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(SFML_USE_STATIC_STD_LIBS "Use static libraries" ON)
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(BOX2D_BUILD_UNIT_TESTS "Build the Box2D unit tests" OFF)
option(BOX2D_BUILD_TESTBED "Build the Box2D testbed" OFF)
option(BOX2D_USER_SETTINGS "Override Box2D settings with b2UserSettings.h" OFF)

include(FetchContent)
FetchContent_Declare(SFML GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.6.x)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git)
FetchContent_Declare(imgui-sfml GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git GIT_TAG 2.6.x)
FetchContent_Declare(box2d GIT_REPOSITORY https://github.com/erincatto/box2d.git GIT_TAG main)
FetchContent_Declare(entt GIT_REPOSITORY https://github.com/skypjack/entt.git GIT_TAG v3.8.x)
FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG master)
FetchContent_MakeAvailable(SFML)
FetchContent_MakeAvailable(imgui)
FetchContent_MakeAvailable(box2d)
FetchContent_MakeAvailable(entt)
FetchContent_MakeAvailable(nlohmann_json)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
FetchContent_MakeAvailable(imgui-sfml)

if(WIN32)
	set(SFML_LIBS sfml-graphics sfml-window sfml-audio sfml-network sfml-main sfml-system -static)
elseif(LINUX)
    set(SFML_LIBS sfml-graphics sfml-window sfml-audio sfml-network sfml-system)
endif()

set(IMGUI_DIR ${imgui_SOURCE_DIR})
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
FetchContent_MakeAvailable(imgui-sfml)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SFML_LIBS} ImGui-SFML::ImGui-SFML EnTT::EnTT box2d nlohmann_json::nlohmann_json)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)


if(WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        VERBATIM)
        
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_link_options(${PROJECT_NAME} PRIVATE "-mwindows")
    endif()
endif()

add_custom_command(
    TARGET ${PROJECT_NAME}
    COMMENT "Copy Assets"
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    VERBATIM)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()
