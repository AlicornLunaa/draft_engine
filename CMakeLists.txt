# Engine library
cmake_minimum_required(VERSION 3.8)
project("draft_engine" VERSION 0.0.1 LANGUAGES CXX)

set(SOURCES
    src/draft/core/application.cpp
    src/draft/core/scene.cpp
    src/draft/core/entity.cpp
    src/draft/input/keyboard.cpp
    src/draft/input/mouse.cpp
    src/draft/phys/collider.cpp
    src/draft/phys/conversions.cpp
    src/draft/phys/fixture.cpp
    src/draft/phys/joint.cpp
    src/draft/phys/rigid_body.cpp
    src/draft/phys/shape.cpp
    src/draft/phys/world.cpp
    src/draft/rendering/render_window.cpp
    src/draft/rendering/sprite_batch.cpp
    src/draft/rendering/shape_batch.cpp
    src/draft/rendering/shader.cpp
    src/draft/rendering/model.cpp
    src/draft/rendering/camera.cpp
    src/draft/rendering/vertex_buffer.cpp
    src/draft/rendering/texture.cpp
    src/draft/rendering/mesh.cpp
    src/draft/rendering/material.cpp
    src/draft/util/file_handle.cpp
    src/draft/util/logger.cpp
    src/draft/util/json.cpp
    src/draft/util/asset_manager.cpp
    src/draft/util/time.cpp
    src/draft/util/clock.cpp
    src/draft/widgets/console.cpp
    src/draft/widgets/stats.cpp
    src/draft/systems/render_system.cpp
    src/draft/systems/physics_system.cpp
)

set(HEADERS
    src/draft/math/vector2_p.hpp
    src/draft/math/vector3_p.hpp
    src/draft/phys/conversions_p.hpp
)

set(INTERFACES
    include/draft/components/control_component.hpp
    include/draft/components/empty_component.hpp
    include/draft/components/texture_component.hpp
    include/draft/components/sprite_component.hpp
    include/draft/components/transform_component.hpp
    include/draft/core/application.hpp
    include/draft/core/scene.hpp
    include/draft/core/entity.hpp
    include/draft/input/keys.hpp
    include/draft/input/event.hpp
    include/draft/input/keyboard.hpp
    include/draft/input/mouse.hpp
    include/draft/math/vector2.hpp
    include/draft/math/vector3.hpp
    include/draft/math/vector4.hpp
    include/draft/math/matrix.hpp
    include/draft/math/rect.hpp
    include/draft/phys/body_def.hpp
    include/draft/phys/collider.hpp
    include/draft/phys/filter.hpp
    include/draft/phys/fixture_def.hpp
    include/draft/phys/fixture.hpp
    include/draft/phys/joint_def.hpp
    include/draft/phys/joint.hpp
    include/draft/phys/rigid_body.hpp
    include/draft/phys/shape.hpp
    include/draft/phys/world.hpp
    include/draft/rendering/camera.hpp
    include/draft/rendering/mesh.hpp
    include/draft/rendering/model.hpp
    include/draft/rendering/render_window.hpp
    include/draft/rendering/sprite_batch.hpp
    include/draft/rendering/shape_batch.hpp
    include/draft/rendering/shader.hpp
    include/draft/rendering/material.hpp
    include/draft/rendering/vertex_buffer.hpp
    include/draft/rendering/texture.hpp
    include/draft/util/file_handle.hpp
    include/draft/util/ansi_colors.hpp
    include/draft/util/circular_buffer.hpp
    include/draft/util/asset_manager.hpp
    include/draft/util/constants.hpp
    include/draft/util/logger.hpp
    include/draft/util/json.hpp
    include/draft/util/time.hpp
    include/draft/util/clock.hpp
    include/draft/widgets/console.hpp
    include/draft/widgets/stats.hpp
    include/draft/systems.hpp
)

set(RESOURCES
    assets/missing_texture.png
    assets/empty_normal_map.png
    assets/debug_white.png
    assets/debug_black.png
    assets/missing_model.glb
)

set(SFML_STATIC_LIBRARIES TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLAD_PROFILE "core"	CACHE STRING "OpenGL profile")
set(GLAD_API "gl=" CACHE STRING "API type/version pairs, like \"gl=3.2,gles=\", no version means latest")
set(GLAD_GENERATOR "c"	CACHE STRING "Language to generate the binding for")
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(SFML_USE_STATIC_STD_LIBS "Use static libraries" ON)
option(BOX2D_BUILD_UNIT_TESTS "Build the Box2D unit tests" OFF)
option(BOX2D_BUILD_TESTBED "Build the Box2D testbed" OFF)
option(BOX2D_USER_SETTINGS "Override Box2D settings with b2UserSettings.h" OFF)

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(SFML GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.6.x)
FetchContent_Declare(box2d GIT_REPOSITORY https://github.com/erincatto/box2d.git GIT_TAG v2.4.1)
FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG master)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(SFML)
FetchContent_MakeAvailable(box2d)
FetchContent_MakeAvailable(nlohmann_json)

add_subdirectory(vendor/glad)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/stb)
add_subdirectory(vendor/entt)

if(WIN32)
	set(SFML_FLAGS sfml-graphics sfml-window sfml-audio sfml-network sfml-main sfml-system -static)
elseif(LINUX)
    set(SFML_FLAGS sfml-graphics sfml-window sfml-audio sfml-network sfml-system)
endif()

# Resource compiler
file(DOWNLOAD "https://raw.githubusercontent.com/vector-of-bool/cmrc/master/CMakeRC.cmake" "${CMAKE_BINARY_DIR}/CMakeRC.cmake")
include("${CMAKE_BINARY_DIR}/CMakeRC.cmake")
cmrc_add_resource_library(
    ${PROJECT_NAME}-resources
    ALIAS ${PROJECT_NAME}::rc
    NAMESPACE ${PROJECT_NAME}
    ${RESOURCES}
)

add_library(${PROJECT_NAME} STATIC)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinygltf
        
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME} PUBLIC entt box2d)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad imgui stb nlohmann_json::nlohmann_json ${SFML_FLAGS} ${PROJECT_NAME}::rc)
# target_link_libraries(${PROJECT_NAME} PUBLIC entt)
# target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad box2d imgui stb nlohmann_json::nlohmann_json ${SFML_FLAGS} ${PROJECT_NAME}::rc)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Engine tools
set(TOOL_SOURCES
    src/tools/main.cpp
)

project("draft_engine_tools" VERSION 0.0.1 LANGUAGES CXX)
add_executable(${PROJECT_NAME} ${TOOL_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src/tools)
target_link_libraries(${PROJECT_NAME} PRIVATE draft_engine::draft_engine stb)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)