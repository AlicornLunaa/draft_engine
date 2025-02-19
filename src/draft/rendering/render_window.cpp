#define GLFW_INCLUDE_NONE

#include "draft/rendering/render_window.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include "tracy/TracyOpenGL.hpp"

#include <string>

using namespace std;

namespace Draft {
    // Constructors
    RenderWindow::RenderWindow(unsigned int width, unsigned int height, const string& title) : Window(width, height, title) {
        // Start GLAD
        if(!gladLoadGL(glfwGetProcAddress)){
            Logger::println(Level::CRITICAL, "GLAD", "Failed to initialize");
            exit(0);
        }

        // Setup opengl context
        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Start gpu profiler
        TracyGpuContext;
    }

    // Functions
    void RenderWindow::set_viewport(const Vector2f& position, const Vector2f& size) const {
        glViewport(position.x, position.y, size.x, size.y);
    }

    void RenderWindow::set_viewport(const Vector2f& size) const {
        glViewport(0, 0, size.x, size.y);
    }

    void RenderWindow::clear(const Vector4f& clearColor){
        // Profiler frame start
        TracyGpuZone("window_clear");

        // Clear window
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void RenderWindow::display(){
        // End gpu frame
        TracyGpuZone("window_display");

        // Finalize frame
        swap_buffers();

        // Collect profiler data
        TracyGpuCollect;
    }
};