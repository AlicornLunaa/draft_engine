#define GLFW_INCLUDE_VULKAN

#include <format>

#include "draft/rendering/render_window.hpp"
#include "draft/util/logger.hpp"
#include "GLFW/glfw3.h"

using namespace std;

namespace Draft {
    // Pimpl declaration
    struct RenderWindow::Impl {
        // Variables
        GLFWwindow* window = nullptr;
    };

    // Definitions
    RenderWindow::RenderWindow(unsigned int width, unsigned int height, const string& title) : ptr(std::make_unique<Impl>()) {
        // Callbacks
        glfwSetErrorCallback([](int errorCode, const char* errorDesc){
            Logger::println(Level::CRITICAL, "GLFW", format("{}, code: {}", errorDesc, errorCode));
        });

        // Start GLFW
        if(glfwInit() == GLFW_FALSE)
            exit(0);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        ptr->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    }

    RenderWindow::~RenderWindow(){
        glfwDestroyWindow(ptr->window);
        glfwTerminate();
    }

    // Functions
    bool RenderWindow::is_open(){ return !glfwWindowShouldClose(ptr->window); }
    void RenderWindow::poll_events(){ glfwPollEvents(); }
};