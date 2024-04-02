#define GLFW_INCLUDE_NONE

#include <format>

#include "draft/rendering/render_window.hpp"
#include "draft/util/logger.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace std;

namespace Draft {
    // Pimpl declaration
    struct RenderWindow::Impl {
        // Variables
        GLFWwindow* window = nullptr;

        // Constructors
        Impl(unsigned int w, unsigned int h, const string& title){
            // Callbacks
            glfwSetErrorCallback([](int errorCode, const char* errorDesc){
                Logger::println(Level::CRITICAL, "GLFW", format("{}, code: {}", errorDesc, errorCode));
            });

            // Start GLFW
            if(glfwInit() == GLFW_FALSE)
                exit(0);

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
            glfwMakeContextCurrent(window);

            // Start GLAD
            if(!gladLoadGL(glfwGetProcAddress)){
                Logger::println(Level::CRITICAL, "GLAD", "Failed to initialize");
                exit(0);
            }

            // Setup opengl context
            glViewport(0, 0, w, h);
        }

        ~Impl(){
            glfwDestroyWindow(window);
            glfwTerminate();
        }
    };

    // Definitions
    RenderWindow::RenderWindow(unsigned int width, unsigned int height, const string& title) : ptr(std::make_unique<Impl>(width, height, title)) {}

    RenderWindow::~RenderWindow(){}

    // Functions
    bool RenderWindow::is_open(){ return !glfwWindowShouldClose(ptr->window); }
    void RenderWindow::swap_buffers(){ glfwSwapBuffers(ptr->window); }
    void RenderWindow::poll_events(){ glfwPollEvents(); }

    void RenderWindow::render(){
        glClearColor(0.2, 0.3, 0.4, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};