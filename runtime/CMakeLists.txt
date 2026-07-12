cmake_minimum_required(VERSION 3.11)
project("draft_runtime" VERSION 1.0.0 LANGUAGES CXX)

option(DRAFT_RUNTIME_BUILD_TESTS "Build the tests for draft_runtime" ON)

include(FetchContent)
FetchContent_Declare(entt GIT_REPOSITORY https://github.com/skypjack/entt.git GIT_TAG v3.13.2)
FetchContent_MakeAvailable(entt)
get_target_property(ENTT_INCLUDE_DIRS EnTT INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(EnTT PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${ENTT_INCLUDE_DIRS}")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_MakeAvailable(glfw)

# Only the Audio module, Runtime has its own GLFW-based Window, and no use for SFML's Window/Graphics/Network modules.
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(SFML_USE_STATIC_STD_LIBS "Use static libraries" ON)
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
set(SFML_BUILD_WINDOW OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_GRAPHICS OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SFML GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_USER_SETTINGS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(box2d GIT_REPOSITORY https://github.com/erincatto/box2d.git GIT_TAG v2.4.1)
FetchContent_MakeAvailable(box2d)

FetchContent_Declare(mimalloc GIT_REPOSITORY https://github.com/microsoft/mimalloc.git GIT_TAG v3.0.3)
FetchContent_MakeAvailable(mimalloc)

FetchContent_Declare(rmlui GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG master)
FetchContent_MakeAvailable(rmlui)
get_target_property(RMLUI_INCLUDE_DIRS rmlui INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(rmlui PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${RMLUI_INCLUDE_DIRS}")

FetchContent_Declare(tracy GIT_REPOSITORY https://github.com/wolfpld/tracy.git GIT_TAG v0.13.0)
FetchContent_MakeAvailable(tracy)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TRACY_ENABLE ON CACHE BOOL "Disable Tracy in release" FORCE)
else()
    set(TRACY_ENABLE OFF CACHE BOOL "Disable Tracy in release" FORCE)
endif()

add_subdirectory(vendor/glad)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/stb)
add_subdirectory(vendor/tinygltf)
add_subdirectory(vendor/opensimplexnoise)

find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)

if(WIN32)
    set(SFML_FLAGS sfml-audio user32 gdi32 -static)
    add_compile_definitions(WINDOWS)
elseif(LINUX)
    set(SFML_FLAGS sfml-audio X11 pthread dl)
    add_compile_definitions(UNIX)
endif()

set(HEADERS
    include/draft/aliasing/attachment.hpp
    include/draft/aliasing/filter.hpp
    include/draft/aliasing/format.hpp
    include/draft/aliasing/framebuffer.hpp
    include/draft/aliasing/parameter.hpp
    include/draft/aliasing/target.hpp
    include/draft/aliasing/wrap.hpp
    include/draft/asset/asset_manager.hpp
    include/draft/asset/resource.hpp
    include/draft/audio/listener.hpp
    include/draft/audio/music.hpp
    include/draft/audio/sound.hpp
    include/draft/audio/sound_buffer.hpp
    include/draft/components/audio_components.hpp
    include/draft/components/collider_component.hpp
    include/draft/components/joint_component.hpp
    include/draft/components/rigid_body_component.hpp
    include/draft/components/sprite_component.hpp
    include/draft/components/tag_component.hpp
    include/draft/components/transform_component.hpp
    include/draft/core/application.hpp
    include/draft/ecs/audio_system.hpp
    include/draft/ecs/entity.hpp
    include/draft/ecs/physics_system.hpp
    include/draft/ecs/registry.hpp
    include/draft/ecs/relationship_components.hpp
    include/draft/ecs/relationship_system.hpp
    include/draft/ecs/render_system.hpp
    include/draft/ecs/scene.hpp
    include/draft/ecs/system.hpp
    include/draft/input/action.hpp
    include/draft/input/event.hpp
    include/draft/input/keyboard.hpp
    include/draft/input/mouse.hpp
    include/draft/phys/body_def.hpp
    include/draft/phys/body_type.hpp
    include/draft/phys/collider.hpp
    include/draft/phys/filter.hpp
    include/draft/phys/fixture.hpp
    include/draft/phys/fixture_def.hpp
    include/draft/phys/joint.hpp
    include/draft/phys/joint_data.hpp
    include/draft/phys/joint_def.hpp
    include/draft/phys/mass_data.hpp
    include/draft/phys/raycast_props.hpp
    include/draft/phys/rigid_body.hpp
    include/draft/phys/shapes/chain_shape.hpp
    include/draft/phys/shapes/circle_shape.hpp
    include/draft/phys/shapes/edge_shape.hpp
    include/draft/phys/shapes/polygon_shape.hpp
    include/draft/phys/shapes/shape.hpp
    include/draft/phys/world.hpp
    include/draft/rendering/batching/collection.hpp
    include/draft/rendering/batching/draw_command.hpp
    include/draft/rendering/batching/shape_collection.hpp
    include/draft/rendering/batching/shape_point.hpp
    include/draft/rendering/batching/sprite_collection.hpp
    include/draft/rendering/batching/sprite_props.hpp
    include/draft/rendering/batching/text_renderer.hpp
    include/draft/rendering/animation.hpp
    include/draft/rendering/camera.hpp
    include/draft/rendering/clip.hpp
    include/draft/rendering/font.hpp
    include/draft/rendering/frame_buffer.hpp
    include/draft/rendering/image.hpp
    include/draft/rendering/material.hpp
    include/draft/rendering/mesh.hpp
    include/draft/rendering/model.hpp
    include/draft/rendering/particle_system.hpp
    include/draft/rendering/pipeline/passes/composite_pass.hpp
    include/draft/rendering/pipeline/passes/geometry_pass.hpp
    include/draft/rendering/pipeline/passes/interface_pass.hpp
    include/draft/rendering/pipeline/render_pass.hpp
    include/draft/rendering/pipeline/renderer.hpp
    include/draft/rendering/render_state.hpp
    include/draft/rendering/render_window.hpp
    include/draft/rendering/shader.hpp
    include/draft/rendering/shader_buffer.hpp
    include/draft/rendering/texture.hpp
    include/draft/rendering/texture_packer.hpp
    include/draft/rendering/vertex_array.hpp
    include/draft/rendering/window.hpp
)

set(SOURCES
    src/draft/aliasing/format.cpp
    src/draft/asset/asset_manager.cpp
    src/draft/audio/listener.cpp
    src/draft/audio/music.cpp
    src/draft/audio/sound.cpp
    src/draft/audio/sound_buffer.cpp
    src/draft/core/application.cpp
    src/draft/ecs/audio_system.cpp
    src/draft/ecs/entity.cpp
    src/draft/ecs/physics_system.cpp
    src/draft/ecs/relationship_system.cpp
    src/draft/ecs/render_system.cpp
    src/draft/ecs/scene.cpp
    src/draft/ecs/system.cpp
    src/draft/input/keyboard.cpp
    src/draft/input/mouse.cpp
    src/draft/phys/b2_raycast_proxy.cpp
    src/draft/phys/collider.cpp
    src/draft/phys/conversions.cpp
    src/draft/phys/fixture.cpp
    src/draft/phys/joint.cpp
    src/draft/phys/rigid_body.cpp
    src/draft/phys/shapes/chain_shape.cpp
    src/draft/phys/shapes/circle_shape.cpp
    src/draft/phys/shapes/edge_shape.cpp
    src/draft/phys/shapes/polygon_shape.cpp
    src/draft/phys/world.cpp
    src/draft/rendering/batching/collection.cpp
    src/draft/rendering/batching/shape_collection.cpp
    src/draft/rendering/batching/sprite_collection.cpp
    src/draft/rendering/batching/text_renderer.cpp
    src/draft/rendering/animation.cpp
    src/draft/rendering/camera.cpp
    src/draft/rendering/clip.cpp
    src/draft/rendering/font.cpp
    src/draft/rendering/frame_buffer.cpp
    src/draft/rendering/image.cpp
    src/draft/rendering/material.cpp
    src/draft/rendering/mesh.cpp
    src/draft/rendering/model.cpp
    src/draft/rendering/particle_system.cpp
    src/draft/rendering/phys_renderer_p.cpp
    src/draft/rendering/pipeline/passes/composite_pass.cpp
    src/draft/rendering/pipeline/passes/geometry_pass.cpp
    src/draft/rendering/pipeline/passes/interface_pass.cpp
    src/draft/rendering/pipeline/render_pass.cpp
    src/draft/rendering/pipeline/renderer.cpp
    src/draft/rendering/render_window.cpp
    src/draft/rendering/shader.cpp
    src/draft/rendering/stb_image_impl.cpp
    src/draft/rendering/texture.cpp
    src/draft/rendering/texture_packer.cpp
    src/draft/rendering/vertex_array.cpp
    src/draft/rendering/window.cpp
)

add_library(${PROJECT_NAME} STATIC)
add_library(draft::runtime ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        draft::common
        EnTT::EnTT
        glad # should be fixed later by manual aliasing functions
        imgui
        TracyClient
        RmlUi::RmlUi
        simplex_noise
    PRIVATE
        Threads::Threads
        glfw
        stb
        tinygltf
        Freetype::Freetype
        box2d
        ${SFML_FLAGS}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:TRACY_ENABLE>)

if(DRAFT_RUNTIME_BUILD_TESTS)
    message(STATUS "Building draft_runtime tests")
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip)
    FetchContent_MakeAvailable(googletest)

    file(GLOB_RECURSE RUNTIME_TEST_SOURCES "tests/**/*.cpp")

    add_executable(
        draft_runtime_tests
        ${RUNTIME_TEST_SOURCES}
    )

    target_link_libraries(
        draft_runtime_tests
        PRIVATE
        GTest::gtest_main
        ${PROJECT_NAME}
        glfw
        glad
        ${SFML_FLAGS}
    )

    include(GoogleTest)
    gtest_discover_tests(draft_runtime_tests)
endif()
