#include "draft/rendering/window.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"

#include <stdexcept>

namespace Draft {
    // Statics
    uint Window::s_glfwCount = 0;
    bool Window::s_glfwErrorCallback = false;

    // Private callback handlers
    void Window::window_resize_callback(GLFWwindow* window, int width, int height){
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if(d_window && d_window->sizeCallback){
            d_window->sizeCallback(width, height);
        }
    }

    void Window::window_resize_framebuffer_callback(GLFWwindow* window, int width, int height){
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if(d_window && d_window->frameSizeCallback){
            d_window->frameSizeCallback(width, height);
        }
    }

    void Window::window_focus_callback(GLFWwindow* window, int focused){
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if(d_window && d_window->focusCallback){
            d_window->focusCallback(focused == GLFW_FOCUSED);
        }
    }

    void Window::window_close_callback(GLFWwindow* window){
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if(d_window && d_window->closeCallback){
            d_window->closeCallback();
        }
    }

    // Protected functions
    GLFWwindow* Window::get_glfw_handle() const {
        return m_window;
    }

    // Constructors
    Window::Window(unsigned int width, unsigned int height, const std::string& title, const GLFWProperties& props){
        // Startup GLFW
        if(!s_glfwErrorCallback){
            // No callback was set, create it here
            glfwSetErrorCallback([](int errorCode, const char* errorDesc){ Logger::println(Level::CRITICAL, "GLFW", std::string(errorDesc) + ", code: " + std::to_string(errorCode)); });
            s_glfwErrorCallback = true;
        }

        if(s_glfwCount == 0){
            // No GLFW exists, start it up
            if(glfwInit() == GLFW_FALSE){
                // GLFW failed to initialize, fail out
                throw std::runtime_error("Failed to initialize GLFW");
            }
        }

        // Setup window hints
        bool noContext = false;

        for(auto& [hint, value] : props){
            if(hint == GLFW_CLIENT_API && value == GLFW_NO_API)
                noContext = true;

            glfwWindowHint(hint, value);
        }

        // Debug hint
        #ifdef TRACY_ENABLE
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        #else
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
        #endif

        // Create new window with the current hints and setup callbacks
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if(!m_window){
            throw std::runtime_error("Failed to create GLFW window");
        }
        
        glfwSetWindowUserPointer(m_window, static_cast<void*>(this)); // Makes it possible to convert glfw -> draft
        
        if(!noContext){
            // These functions need a context, theyre ignored if no context specified
            glfwMakeContextCurrent(m_window);
            set_vsync(true);
        }
        
        // Callbacks
        glfwSetWindowSizeCallback(m_window, Window::window_resize_callback);
        glfwSetFramebufferSizeCallback(m_window, Window::window_resize_framebuffer_callback);
        glfwSetWindowFocusCallback(m_window, Window::window_focus_callback);
        glfwSetWindowCloseCallback(m_window, Window::window_close_callback);
        
        // Increment count because another instance is now using glfw
        s_glfwCount++;
    }

    Window::Window(Window&& other) noexcept : m_window(other.m_window), m_keyboard(other.m_keyboard), m_mouse(other.m_mouse), m_vsyncEnabled(other.m_vsyncEnabled) {
        other.m_window = nullptr;
        other.m_keyboard = nullptr;
        other.m_mouse = nullptr;
        other.m_vsyncEnabled = false;
    }

    Window::~Window(){
        // Cleanup keyboard and mouse
        if(m_keyboard){
            m_keyboard->m_window = nullptr;
        }

        if(m_mouse){
            m_mouse->m_window = nullptr;
        }

        // Close window
        glfwDestroyWindow(m_window);
        s_glfwCount--;

        // Cleanup GLFW if this was the last window
        if(s_glfwCount == 0){
            glfwTerminate();
        }
    }

    // Operators
    Window& Window::operator=(Window&& other) noexcept {
        if(&other != this){
            this->m_window = other.m_window;
            this->m_keyboard = other.m_keyboard;
            this->m_mouse = other.m_mouse;
            this->m_vsyncEnabled = other.m_vsyncEnabled;
            other.m_window = nullptr;
            other.m_keyboard = nullptr;
            other.m_mouse = nullptr;
            other.m_vsyncEnabled = false;
        }

        return *this;
    }

    // Functions
    void Window::poll_events() const {
        glfwPollEvents();
    }

    void Window::swap_buffers() const {
        glfwSwapBuffers(m_window);
    }

    void Window::close(){
        set_open(false);
    }

    void Window::set_fullscreen(bool flag){
        GLFWmonitor* monitor = flag ? glfwGetPrimaryMonitor() : nullptr;
        auto size = get_size();
        glfwSetWindowMonitor(m_window, monitor, 0, 0, size.x, size.y, GLFW_DONT_CARE);
    }

    void Window::set_title(const std::string& title){
        glfwSetWindowTitle(m_window, title.c_str());
    }

    void Window::set_position(const Vector2i& position){
        glfwSetWindowPos(m_window, position.x, position.y);
    }

    void Window::set_size(const Vector2u& size){
        glfwSetWindowSize(m_window, size.x, size.y);
    }

    void Window::set_vsync(bool flag){
        glfwSwapInterval(flag ? 1 : 0);
        m_vsyncEnabled = flag;
    }

    void Window::set_open(bool flag){
        glfwSetWindowShouldClose(m_window, !flag);
    }

    void Window::set_icon(const Image& image){
        exit(500);
    }

    void Window::reset_icon(){
        glfwSetWindowIcon(m_window, 0, nullptr);
    }
        
    const std::string Window::get_title() const {
        return glfwGetWindowTitle(m_window);
    }

    const Vector2i Window::get_position() const {
        Vector2i pos(0, 0);
        glfwGetWindowPos(m_window, &pos.x, &pos.y);
        return pos;
    }

    const Vector2u Window::get_size() const {
        Vector2i size(0, 0);
        glfwGetWindowSize(m_window, &size.x, &size.y);
        return size;
    }

    const Vector2u Window::get_frame_size() const {
        Vector2i size(0, 0);
        glfwGetFramebufferSize(m_window, &size.x, &size.y);
        return size;
    }

    bool Window::is_fullscreen() const {
        return glfwGetWindowMonitor(m_window) != nullptr;
    }

    bool Window::is_vsync() const {
        return m_vsyncEnabled;
    }

    bool Window::is_open() const {
        return !glfwWindowShouldClose(m_window);
    }

    // Static functions
    GLFWProperties Window::get_default_properties(){
        GLFWProperties props = {
            {GLFW_CONTEXT_VERSION_MAJOR, 4},
            {GLFW_CONTEXT_VERSION_MINOR, 5},
            {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE},
        };

        return props;
    }
}