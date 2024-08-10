# Engine library
cmake_minimum_required(VERSION 3.8)
project("draft_engine" VERSION 0.1.0 LANGUAGES CXX)

set(SOURCES
    src/draft/audio/sound_buffer.cpp
    src/draft/audio/sound.cpp
    src/draft/audio/music.cpp
    src/draft/audio/listener.cpp
    src/draft/core/application.cpp
    src/draft/core/scene.cpp
    src/draft/core/entity.cpp
    src/draft/input/keyboard.cpp
    src/draft/input/mouse.cpp
    src/draft/interface/panel.cpp
    src/draft/interface/ui_container.cpp
    src/draft/interface/rectangle.cpp
    src/draft/interface/slider.cpp
    src/draft/interface/button.cpp
    src/draft/interface/joystick.cpp
    src/draft/interface/picture.cpp
    src/draft/interface/progress_bar.cpp
    src/draft/interface/widgets/console.cpp
    src/draft/interface/widgets/stats.cpp
    src/draft/math/bounds.cpp
    src/draft/phys/collider.cpp
    src/draft/phys/conversions.cpp
    src/draft/phys/fixture.cpp
    src/draft/phys/joint.cpp
    src/draft/phys/rigid_body.cpp
    src/draft/phys/shapes/polygon_shape.cpp
    src/draft/phys/shapes/edge_shape.cpp
    src/draft/phys/shapes/circle_shape.cpp
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
    src/draft/rendering/font.cpp
    src/draft/rendering/phys_renderer_p.cpp
    src/draft/rendering/text_renderer.cpp
    src/draft/rendering/image.cpp
    src/draft/rendering/conversions.cpp
    src/draft/rendering/particle_system.cpp
    src/draft/rendering/animation.cpp
    src/draft/util/file_handle.cpp
    src/draft/util/logger.cpp
    src/draft/util/json.cpp
    src/draft/util/asset_manager.cpp
    src/draft/util/asset_loaders.cpp
    src/draft/util/time.cpp
    src/draft/util/clock.cpp
    src/draft/systems/render_system.cpp
    src/draft/systems/physics_system.cpp
)

set(HEADERS
    src/draft/math/vector2_p.hpp
    src/draft/math/vector3_p.hpp
    src/draft/phys/conversions_p.hpp
    src/draft/rendering/phys_renderer_p.hpp
    src/draft/rendering/conversions_p.hpp
)

set(INTERFACES
    include/draft/audio/sound_buffer.hpp
    include/draft/audio/sound.hpp
    include/draft/audio/music.hpp
    include/draft/audio/listener.hpp
    include/draft/components/control_component.hpp
    include/draft/components/empty_component.hpp
    include/draft/components/texture_component.hpp
    include/draft/components/sprite_component.hpp
    include/draft/components/transform_component.hpp
    include/draft/components/collider_component.hpp
    include/draft/core/application.hpp
    include/draft/core/scene.hpp
    include/draft/core/entity.hpp
    include/draft/input/action.hpp
    include/draft/input/event.hpp
    include/draft/input/keyboard.hpp
    include/draft/input/mouse.hpp
    include/draft/interface/panel.hpp
    include/draft/interface/ui_container.hpp
    include/draft/interface/rectangle.hpp
    include/draft/interface/slider.hpp
    include/draft/interface/button.hpp
    include/draft/interface/joystick.hpp
    include/draft/interface/picture.hpp
    include/draft/interface/progress_bar.hpp
    include/draft/interface/widgets/console.hpp
    include/draft/interface/widgets/stats.hpp
    include/draft/math/funcs.hpp
    include/draft/math/bounds.hpp
    include/draft/math/rect.hpp
    include/draft/math/glm.hpp
    include/draft/phys/body_def.hpp
    include/draft/phys/collider.hpp
    include/draft/phys/filter.hpp
    include/draft/phys/fixture_def.hpp
    include/draft/phys/fixture.hpp
    include/draft/phys/joint_def.hpp
    include/draft/phys/joint.hpp
    include/draft/phys/rigid_body.hpp
    include/draft/phys/world.hpp
    include/draft/phys/shapes/shape.hpp
    include/draft/phys/shapes/polygon_shape.hpp
    include/draft/phys/shapes/edge_shape.hpp
    include/draft/phys/shapes/circle_shape.hpp
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
    include/draft/rendering/font.hpp
    include/draft/rendering/text_renderer.hpp
    include/draft/rendering/image.hpp
    include/draft/rendering/animation.hpp
    include/draft/systems/physics_system.hpp
    include/draft/systems/render_system.hpp
    include/draft/util/file_handle.hpp
    include/draft/util/ansi_colors.hpp
    include/draft/util/circular_buffer.hpp
    include/draft/util/asset_manager.hpp
    include/draft/util/asset_loaders.hpp
    include/draft/util/logger.hpp
    include/draft/util/json.hpp
    include/draft/util/time.hpp
    include/draft/util/clock.hpp
)

set(RESOURCES
    assets/textures/missing_texture.png
    assets/textures/empty_normal_map.png
    assets/textures/debug_white.png
    assets/textures/debug_black.png

    assets/models/missing_model.glb

    assets/fonts/missing_font.ttf

    assets/shaders/missing_shader/vertex.glsl
    assets/shaders/missing_shader/fragment.glsl
    assets/shaders/default/vertex.glsl
    assets/shaders/default/fragment.glsl
    assets/shaders/interface/vertex.glsl
    assets/shaders/interface/fragment.glsl
    assets/shaders/mesh/vertex.glsl
    assets/shaders/mesh/fragment.glsl
    assets/shaders/shapes/vertex.glsl
    assets/shaders/shapes/fragment.glsl
    assets/shaders/text/vertex.glsl
    assets/shaders/text/fragment.glsl
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
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEV_BUILD")
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(SFML_USE_STATIC_STD_LIBS "Use static libraries" ON)
option(BOX2D_BUILD_UNIT_TESTS "Build the Box2D unit tests" OFF)
option(BOX2D_BUILD_TESTBED "Build the Box2D testbed" OFF)
option(BOX2D_USER_SETTINGS "Override Box2D settings with b2UserSettings.h" OFF)

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(SFML GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.6.x)
FetchContent_Declare(box2d GIT_REPOSITORY https://github.com/erincatto/box2d.git GIT_TAG v2.4.1)
FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG master)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(SFML)
FetchContent_MakeAvailable(box2d)
FetchContent_MakeAvailable(nlohmann_json)

add_subdirectory(vendor/glad)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/stb)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/opensimplexnoise)
find_package(Freetype REQUIRED)

if(WIN32)
	set(SFML_FLAGS sfml-audio sfml-network sfml-main sfml-system -static)
elseif(LINUX)
    set(SFML_FLAGS sfml-audio sfml-network sfml-system)
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
        ${FREETYPE_INCLUDE_DIRS}
        
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME} PUBLIC entt imgui glm::glm nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad box2d stb simplex_noise ${FREETYPE_LIBRARIES} ${SFML_FLAGS} ${PROJECT_NAME}::rc)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)