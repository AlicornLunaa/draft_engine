cmake_minimum_required(VERSION 3.11)
project("draft_common" VERSION 1.0.0 LANGUAGES CXX)

option(DRAFT_COMMON_BUILD_TESTS "Build the tests for draft_common" ON)

include(FetchContent)
FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG master)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glm)

set(HEADERS
    include/draft/math/bounds.hpp
    include/draft/math/color.hpp
    include/draft/math/funcs.hpp
    include/draft/math/glm.hpp
    include/draft/math/rect.hpp
    include/draft/util/circular_buffer.hpp
    include/draft/util/clock.hpp
    include/draft/util/files/asset_file_system.hpp
    include/draft/util/files/disk_file_provider.hpp
    include/draft/util/files/embedded_file_provider.hpp
    include/draft/util/files/file_handle.hpp
    include/draft/util/files/file_provider.hpp
    include/draft/util/files/host_file_system.hpp
    include/draft/util/files/memory_file_provider.hpp
    include/draft/util/files/virtual_file_system.hpp
    include/draft/util/json.hpp
    include/draft/util/logger.hpp
    include/draft/util/reflectable.hpp
    include/draft/util/serialization/binary.hpp
    include/draft/util/serialization/context.hpp
    include/draft/util/serialization/serializer.hpp
    include/draft/util/serialization/custom.hpp
    include/draft/util/serialization/stl.hpp
    include/draft/util/serialization/glm.hpp
    include/draft/util/time.hpp
)

set(SOURCES
    src/draft/math/bounds.cpp
    src/draft/math/color.cpp
    src/draft/math/glm.cpp
    src/draft/util/clock.cpp
    src/draft/util/files/asset_file_system.cpp
    src/draft/util/files/disk_file_provider.cpp
    src/draft/util/files/embedded_file_provider.cpp
    src/draft/util/files/file_handle.cpp
    src/draft/util/files/file_provider.cpp
    src/draft/util/files/host_file_system.cpp
    src/draft/util/files/memory_file_provider.cpp
    src/draft/util/files/virtual_file_system.cpp
    src/draft/util/json.cpp
    src/draft/util/logger.cpp
    src/draft/util/time.cpp
)

# Resource compiler, for EmbeddedFileProvider
if(NOT COMMAND cmrc_add_resource_library)
    file(DOWNLOAD "https://raw.githubusercontent.com/vector-of-bool/cmrc/master/CMakeRC.cmake" "${CMAKE_BINARY_DIR}/CMakeRC.cmake")
    include("${CMAKE_BINARY_DIR}/CMakeRC.cmake")
endif()

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
    assets/shaders/geometry/vertex.glsl
    assets/shaders/geometry/fragment.glsl
    assets/shaders/composite/vertex.glsl
    assets/shaders/composite/fragment.glsl
    assets/textures/debug_black.png
    assets/textures/debug_white.png
    assets/textures/empty_normal_map.png
    assets/textures/missing_texture.png
)

cmrc_add_resource_library(
    ${PROJECT_NAME}-resources
    ALIAS ${PROJECT_NAME}::rc
    NAMESPACE draft_engine
    ${RESOURCES}
)

# SHARED in Debug, STATIC in Release.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_library(${PROJECT_NAME} SHARED)
else()
    add_library(${PROJECT_NAME} STATIC)
endif()
add_library(draft::common ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        nlohmann_json::nlohmann_json
        glm::glm
    PRIVATE
        ${PROJECT_NAME}::rc
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:DEBUG>)

if(DRAFT_COMMON_BUILD_TESTS)
    message(STATUS "Building draft_common tests")
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip SYSTEM)
    FetchContent_MakeAvailable(googletest)

    file(GLOB_RECURSE COMMON_TEST_SOURCES "tests/**/*.cpp")

    add_executable(
        draft_common_tests
        ${COMMON_TEST_SOURCES}
    )

    target_link_libraries(
        draft_common_tests
        PRIVATE
        GTest::gtest_main
        ${PROJECT_NAME}
    )

    include(GoogleTest)
    gtest_discover_tests(draft_common_tests)
endif()
