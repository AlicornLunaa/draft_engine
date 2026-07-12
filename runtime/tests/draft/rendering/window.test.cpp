#include <gtest/gtest.h>
#include "draft/rendering/window.hpp"

#include "GLFW/glfw3.h"

#include <algorithm>

TEST(Window, DefaultPropertiesRequestCore4Dot5Context){
    Draft::GLFWProperties props = Draft::Window::get_default_properties();

    EXPECT_NE(std::find(props.begin(), props.end(), std::make_pair(GLFW_CONTEXT_VERSION_MAJOR, 4)), props.end());
    EXPECT_NE(std::find(props.begin(), props.end(), std::make_pair(GLFW_CONTEXT_VERSION_MINOR, 5)), props.end());
    EXPECT_NE(std::find(props.begin(), props.end(), std::make_pair(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE)), props.end());
}
