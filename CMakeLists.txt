cmake_minimum_required(VERSION 3.11)
project("draft" VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# A game module (see build_tools) is a shared library linking draft_common/draft_runtime and
# everything they pull in, so all of that has to be built as position independent code, not just
# the module itself.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

if(MINGW AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    get_filename_component(DRAFT_MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    file(GLOB DRAFT_MINGW_RUNTIME_DLLS
        "${DRAFT_MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
        "${DRAFT_MINGW_BIN_DIR}/libstdc++-6.dll"
        "${DRAFT_MINGW_BIN_DIR}/libwinpthread-1.dll"
    )
    file(COPY ${DRAFT_MINGW_RUNTIME_DLLS} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

add_subdirectory(common)
add_subdirectory(runtime)
add_subdirectory(build_tools)
add_subdirectory(editor)
