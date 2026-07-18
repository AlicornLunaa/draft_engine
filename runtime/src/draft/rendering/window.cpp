#include "draft/rendering/window.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/rendering/image.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"

#include <stdexcept>

namespace Draft {
    unsigned int Window::s_glfwCount = 0;
    bool Window::s_glfwErrorCallback = false;

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

    GLFWwindow* Window::get_glfw_handle() const {
        return m_window;
    }

    Window::Window(unsigned int width, unsigned int height, const std::string& title, const GLFWProperties& props, GLFWwindow* shareContext){
        if(!s_glfwErrorCallback){
            glfwSetErrorCallback([](int errorCode, const char* errorDesc){
                Logger::println(LogLevel::Critical, "GLFW", std::string(errorDesc) + ", code: " + std::to_string(errorCode));
            });
            s_glfwErrorCallback = true;
        }

        if(s_glfwCount == 0){
            if(glfwInit() == GLFW_FALSE){
                throw std::runtime_error("Failed to initialize GLFW");
            }
        }

        bool noContext = false;

        for(auto& [hint, value] : props){
            if(hint == GLFW_CLIENT_API && value == GLFW_NO_API)
                noContext = true;

            glfwWindowHint(hint, value);
        }

        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, shareContext);

        if(!m_window){
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(m_window, static_cast<void*>(this));

        if(!noContext){
            glfwMakeContextCurrent(m_window);
            set_vsync(true);
        }

        glfwSetWindowSizeCallback(m_window, Window::window_resize_callback);
        glfwSetFramebufferSizeCallback(m_window, Window::window_resize_framebuffer_callback);
        glfwSetWindowFocusCallback(m_window, Window::window_focus_callback);
        glfwSetWindowCloseCallback(m_window, Window::window_close_callback);

        s_glfwCount++;
    }

    Window::Window(Window&& other) noexcept : m_window(other.m_window), m_keyboard(other.m_keyboard), m_mouse(other.m_mouse), m_vsyncEnabled(other.m_vsyncEnabled) {
        // Point any inherited Keyboard/Mouse back at this object
        if(m_keyboard) m_keyboard->m_window = static_cast<RenderWindow*>(this);
        if(m_mouse) m_mouse->m_window = static_cast<RenderWindow*>(this);

        other.m_window = nullptr;
        other.m_keyboard = nullptr;
        other.m_mouse = nullptr;
        other.m_vsyncEnabled = false;
    }

    Window::~Window(){
        // Tell any live Keyboard/Mouse their Window just went away, so they don't reach through
        // a dangling pointer on their own next call.
        if(m_keyboard){
            m_keyboard->m_window = nullptr;
        }

        if(m_mouse){
            m_mouse->m_window = nullptr;
        }

        glfwDestroyWindow(m_window);
        s_glfwCount--;

        if(s_glfwCount == 0){
            glfwTerminate();
        }
    }

    Window& Window::operator=(Window&& other) noexcept {
        if(&other != this){
            this->m_window = other.m_window;
            this->m_keyboard = other.m_keyboard;
            this->m_mouse = other.m_mouse;
            this->m_vsyncEnabled = other.m_vsyncEnabled;

            if(m_keyboard) m_keyboard->m_window = static_cast<RenderWindow*>(this);
            if(m_mouse) m_mouse->m_window = static_cast<RenderWindow*>(this);

            other.m_window = nullptr;
            other.m_keyboard = nullptr;
            other.m_mouse = nullptr;
            other.m_vsyncEnabled = false;
        }

        return *this;
    }

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
        Image rgba = image;
        if(rgba.get_format() != ColorFormat::RGBA) rgba.convert(ColorFormat::RGBA);

        GLFWimage icon;
        icon.width = rgba.get_size().x;
        icon.height = rgba.get_size().y;
        icon.pixels = reinterpret_cast<unsigned char*>(const_cast<std::byte*>(rgba.c_arr()));

        glfwSetWindowIcon(m_window, 1, &icon);
    }

    void Window::reset_icon(){
        glfwSetWindowIcon(m_window, 0, nullptr);
    }

    std::string Window::get_title() const {
        return glfwGetWindowTitle(m_window);
    }

    Vector2i Window::get_position() const {
        Vector2i pos(0, 0);
        glfwGetWindowPos(m_window, &pos.x, &pos.y);
        return pos;
    }

    Vector2u Window::get_size() const {
        Vector2i size(0, 0);
        glfwGetWindowSize(m_window, &size.x, &size.y);
        return size;
    }

    Vector2u Window::get_frame_size() const {
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

    GLFWProperties Window::get_default_properties(){
        GLFWProperties props = {
            {GLFW_CONTEXT_VERSION_MAJOR, 4},
            {GLFW_CONTEXT_VERSION_MINOR, 5},
            {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE},
        };

        return props;
    }
}
