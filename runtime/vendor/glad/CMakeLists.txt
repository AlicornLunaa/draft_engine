cmake_minimum_required(VERSION 3.11)
project(glad)

set(GLAD_PROFILE "core"	CACHE STRING "OpenGL profile")
set(GLAD_API "gl=" CACHE STRING "API type/version pairs, like \"gl=3.2,gles=\", no version means latest")
set(GLAD_GENERATOR "c"	CACHE STRING "Language to generate the binding for")

add_library(glad)
target_sources(glad PRIVATE src/gl.c)
target_include_directories(glad PUBLIC include)