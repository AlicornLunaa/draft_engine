#define GLFW_INCLUDE_NONE

#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include <stdexcept>
#include <string>

namespace Draft {
    RenderWindow::RenderWindow(unsigned int width, unsigned int height, const std::string& title, const GLFWProperties& props, GLFWwindow* shareContext) : Window(width, height, title, props, shareContext) {
        if(!gladLoadGL(glfwGetProcAddress)){
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    void RenderWindow::clear(const Vector4f& clearColor){
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void RenderWindow::display(){
        swap_buffers();
    }

    void RenderWindow::resize(const Vector2u& size){
        // Nothing to do, a render window should be explicitly resized using set_size()
    }

    void RenderWindow::begin(const Vector4f& clearColor){
        clear(clearColor);
    }

    void RenderWindow::end(){
        display();
    }
}
