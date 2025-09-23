# Engine library
cmake_minimum_required(VERSION 3.11)
project("draft_engine" VERSION 0.3.0 LANGUAGES CXX)

set(INTERFACES
    include/draft/aliasing/filter.hpp
    include/draft/aliasing/format.hpp
    include/draft/aliasing/parameter.hpp
    include/draft/aliasing/target.hpp
    include/draft/aliasing/wrap.hpp

    include/draft/audio/listener.hpp
    include/draft/audio/music.hpp
    include/draft/audio/sound_buffer.hpp
    include/draft/audio/sound.hpp

    include/draft/components/audio_components.hpp
    include/draft/components/collider_component.hpp
    include/draft/components/empty_component.hpp
    include/draft/components/joint_component.hpp
    include/draft/components/relationship_components.hpp
    include/draft/components/rigid_body_component.hpp
    include/draft/components/sprite_component.hpp
    include/draft/components/tag_component.hpp
    include/draft/components/texture_component.hpp
    include/draft/components/transform_component.hpp

    include/draft/core/application.hpp
    include/draft/core/registry.hpp
    include/draft/core/entity.hpp
    include/draft/core/scene.hpp

    include/draft/input/action.hpp
    include/draft/input/event.hpp
    include/draft/input/keyboard.hpp
    include/draft/input/mouse.hpp
    
    include/draft/interface/clay/clay_interface.hpp
    include/draft/interface/imgui/console.hpp
    include/draft/interface/imgui/imgui_engine.hpp
    include/draft/interface/imgui/stats.hpp
    include/draft/interface/loader/css_loader.hpp
    include/draft/interface/loader/css_style.hpp
    include/draft/interface/rmlui/rml_context.hpp
    include/draft/interface/rmlui/rml_debugger.hpp
    include/draft/interface/rmlui/rml_engine.hpp
    include/draft/interface/rmlui/rml_layout.hpp
    include/draft/interface/rmlui/rml_listener.hpp
    include/draft/interface/rmlui/RmlUi_Include_GL3.h
    include/draft/interface/rmlui/RmlUi_Platform_GLFW.h
    include/draft/interface/rmlui/RmlUi_Renderer_GL3.h
    include/draft/interface/widgets/button.hpp
    include/draft/interface/widgets/grid.hpp
    include/draft/interface/widgets/label.hpp
    include/draft/interface/widgets/layout.hpp
    include/draft/interface/widgets/panel.hpp
    include/draft/interface/widgets/scroll.hpp
    include/draft/interface/command.hpp
    include/draft/interface/context.hpp
    include/draft/interface/dom_tree.hpp
    include/draft/interface/edges.hpp
    include/draft/interface/global_style.hpp
    include/draft/interface/interface.hpp
    include/draft/interface/metrics.hpp
    include/draft/interface/size.hpp
    include/draft/interface/unit_value.hpp

    include/draft/math/bounds.hpp
    include/draft/math/funcs.hpp
    include/draft/math/glm.hpp
    include/draft/math/rect.hpp

    include/draft/phys/shapes/circle_shape.hpp
    include/draft/phys/shapes/edge_shape.hpp
    include/draft/phys/shapes/polygon_shape.hpp
    include/draft/phys/shapes/shape.hpp
    include/draft/phys/mass_data.hpp
    include/draft/phys/body_def.hpp
    include/draft/phys/collider.hpp
    include/draft/phys/filter.hpp
    include/draft/phys/fixture_def.hpp
    include/draft/phys/fixture.hpp
    include/draft/phys/joint_def.hpp
    include/draft/phys/joint.hpp
    include/draft/phys/raycast_props.hpp
    include/draft/phys/rigid_body.hpp
    include/draft/phys/world.hpp

    include/draft/rendering/batching/batch.hpp
    include/draft/rendering/batching/shape_batch.hpp
    include/draft/rendering/batching/sprite_batch.hpp
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
    include/draft/rendering/render_window.hpp
    include/draft/rendering/shader_buffer.hpp
    include/draft/rendering/shader.hpp
    include/draft/rendering/texture_packer.hpp
    include/draft/rendering/texture.hpp
    include/draft/rendering/vertex_array.hpp
    include/draft/rendering/vertex_buffer.hpp
    include/draft/rendering/window.hpp

    include/draft/systems/audio_system.hpp
    include/draft/systems/physics_system.hpp
    include/draft/systems/relationship_system.hpp
    include/draft/systems/render_system.hpp

    include/draft/util/asset_manager/asset_manager.hpp
    include/draft/util/asset_manager/asset_ptr.hpp
    include/draft/util/asset_manager/base_loader.hpp
    include/draft/util/asset_manager/collider_loader.hpp
    include/draft/util/asset_manager/font_loader.hpp
    include/draft/util/asset_manager/generic_loader.hpp
    include/draft/util/asset_manager/image_loader.hpp
    include/draft/util/asset_manager/json_loader.hpp
    include/draft/util/asset_manager/packed_texture_loader.hpp
    include/draft/util/asset_manager/particle_loader.hpp
    include/draft/util/asset_manager/resource.hpp
    include/draft/util/asset_manager/sound_buffer_loader.hpp
    include/draft/util/asset_manager/texture_loader.hpp
    include/draft/util/serialization/serializable.hpp
    include/draft/util/serialization/serializer.hpp
    include/draft/util/serialization/string_serializer.hpp
    include/draft/util/ansi_colors.hpp
    include/draft/util/circular_buffer.hpp
    include/draft/util/clock.hpp
    include/draft/util/color.hpp
    include/draft/util/file_handle.hpp
    include/draft/util/json.hpp
    include/draft/util/logger.hpp
    include/draft/util/reflectable.hpp
    include/draft/util/time.hpp
    include/draft/util/tool_box.hpp
    include/draft/util/types.hpp
)

set(SOURCES
    src/draft/aliasing/format.cpp

    src/draft/audio/listener.cpp
    src/draft/audio/music.cpp
    src/draft/audio/sound_buffer.cpp
    src/draft/audio/sound.cpp

    src/draft/core/application.cpp
    src/draft/core/entity.cpp
    src/draft/core/scene.cpp
    
    src/draft/input/keyboard.cpp
    src/draft/input/mouse.cpp

    src/draft/interface/clay/clay_interface.cpp
    src/draft/interface/imgui/console.cpp
    src/draft/interface/imgui/imgui_engine.cpp
    src/draft/interface/imgui/stats.cpp
    src/draft/interface/loader/css_loader.cpp
    src/draft/interface/rmlui/rml_context.cpp
    src/draft/interface/rmlui/rml_debugger.cpp
    src/draft/interface/rmlui/rml_engine.cpp
    src/draft/interface/rmlui/rml_listener.cpp
    src/draft/interface/rmlui/RmlUi_Platform_GLFW.cpp
    src/draft/interface/rmlui/RmlUi_Renderer_GL3.cpp
    src/draft/interface/widgets/button.cpp
    src/draft/interface/widgets/grid.cpp
    src/draft/interface/widgets/label.cpp
    src/draft/interface/widgets/layout.cpp
    src/draft/interface/widgets/panel.cpp
    src/draft/interface/interface.cpp
    src/draft/interface/unit_value.cpp

    src/draft/math/bounds.cpp
    src/draft/math/glm.cpp

    src/draft/phys/shapes/circle_shape.cpp
    src/draft/phys/shapes/edge_shape.cpp
    src/draft/phys/shapes/polygon_shape.cpp
    src/draft/phys/b2_raycast_proxy.cpp
    src/draft/phys/collider.cpp
    src/draft/phys/conversions.cpp
    src/draft/phys/fixture.cpp
    src/draft/phys/joint.cpp
    src/draft/phys/rigid_body.cpp
    src/draft/phys/world.cpp

    src/draft/rendering/batching/batch.cpp
    src/draft/rendering/batching/shape_batch.cpp
    src/draft/rendering/batching/sprite_batch.cpp
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
    src/draft/rendering/render_window.cpp
    src/draft/rendering/shader_buffer.cpp
    src/draft/rendering/shader.cpp
    src/draft/rendering/texture_packer.cpp
    src/draft/rendering/texture.cpp
    src/draft/rendering/vertex_array.cpp
    src/draft/rendering/vertex_buffer.cpp
    src/draft/rendering/window.cpp

    src/draft/systems/audio_system.cpp
    src/draft/systems/physics_system.cpp
    src/draft/systems/relationship_system.cpp
    src/draft/systems/render_system.cpp

    src/draft/util/asset_manager/asset_manager.cpp
    src/draft/util/asset_manager/collider_loader.cpp
    src/draft/util/asset_manager/font_loader.cpp
    src/draft/util/asset_manager/image_loader.cpp
    src/draft/util/asset_manager/json_loader.cpp
    src/draft/util/asset_manager/packed_texture_loader.cpp
    src/draft/util/asset_manager/particle_loader.cpp
    src/draft/util/asset_manager/sound_buffer_loader.cpp
    src/draft/util/asset_manager/texture_loader.cpp
    src/draft/util/serialization/serializable.cpp
    src/draft/util/serialization/serializer.cpp
    src/draft/util/serialization/string_serializer.cpp
    src/draft/util/clock.cpp
    src/draft/util/color.cpp
    src/draft/util/file_handle.cpp
    src/draft/util/json.cpp
    src/draft/util/logger.cpp
    src/draft/util/time.cpp
    src/draft/util/tool_box.cpp
)

set(HEADERS
    src/draft/math/vector2_p.hpp
    src/draft/math/vector3_p.hpp

    src/draft/phys/b2_raycast_proxy_p.hpp
    src/draft/phys/conversions_p.hpp

    src/draft/rendering/phys_renderer_p.hpp
)

set(RESOURCES
    assets/fonts/missing_font.ttf
    assets/fonts/default.ttf
    assets/fonts/arial.ttf

    assets/models/missing_model.glb

    assets/shaders/default/vertex.glsl
    assets/shaders/default/fragment.glsl
    assets/shaders/mesh/vertex.glsl
    assets/shaders/mesh/fragment.glsl
    assets/shaders/missing_shader/vertex.glsl
    assets/shaders/missing_shader/fragment.glsl
    assets/shaders/shapes/vertex.glsl
    assets/shaders/shapes/fragment.glsl
    assets/shaders/text/vertex.glsl
    assets/shaders/text/fragment.glsl

    assets/textures/debug_black.png
    assets/textures/debug_white.png
    assets/textures/empty_normal_map.png
    assets/textures/missing_texture.png
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
FetchContent_Declare(jemalloc GIT_REPOSITORY https://github.com/jemalloc/jemalloc.git GIT_TAG 5.3.0)
FetchContent_Declare(mimalloc GIT_REPOSITORY https://github.com/microsoft/mimalloc.git GIT_TAG v3.0.3)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(SFML GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.6.x)
FetchContent_Declare(box2d GIT_REPOSITORY https://github.com/erincatto/box2d.git GIT_TAG v2.4.1)
FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG master)
FetchContent_Declare(rmlui GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG 6.0)
FetchContent_Declare(tracy GIT_REPOSITORY https://github.com/wolfpld/tracy.git GIT_TAG v0.12.2)
FetchContent_MakeAvailable(jemalloc)
FetchContent_MakeAvailable(mimalloc)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(SFML)
FetchContent_MakeAvailable(box2d)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(rmlui)
FetchContent_MakeAvailable(tracy)

add_subdirectory(vendor/glad)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/stb)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/opensimplexnoise)
add_subdirectory(vendor/clay)
find_package(Freetype REQUIRED)

if(WIN32)
	set(SFML_FLAGS sfml-audio sfml-network sfml-system -static)
    add_compile_definitions(WINDOWS)
elseif(LINUX)
    set(SFML_FLAGS sfml-audio sfml-network sfml-system)
    add_compile_definitions(UNIX)
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

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glad entt clay imgui glm::glm nlohmann_json::nlohmann_json simplex_noise TracyClient RmlUi::RmlUi)
target_link_libraries(${PROJECT_NAME} PRIVATE box2d stb ${FREETYPE_LIBRARIES} ${SFML_FLAGS} ${PROJECT_NAME}::rc)
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:TRACY_ENABLE> $<$<CONFIG:Debug>:DEBUG>)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)