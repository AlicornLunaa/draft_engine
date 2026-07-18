#include "draft/input/mouse.hpp"
#include "draft/math/rect.hpp"
#include "draft/util/logger.hpp"
#include "draft/rendering/window.hpp"

#include "GLFW/glfw3.h"

#include <cassert>

namespace Draft {
    /// GlfwMouse
    // Static functions
    void GlfwMouse::cleanup_callbacks(GLFWwindow* window){
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetScrollCallback(window, nullptr);
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetCursorEnterCallback(window, nullptr);
    }

    void GlfwMouse::mouse_entered(GLFWwindow* window, int entered){
        // Get current mouse instance
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        Mouse* mouse = d_window->m_mouse;

        // Check for null
        if(mouse == nullptr){
            // Somehow theres a mouse event for a window with no mouse? Attempt recovery and cleanup
            Logger::println(LogLevel::Critical, "Mouse", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Create event for the engine
        if(entered && mouse->mouseEnterCallback){
            mouse->mouseEnterCallback();
        } else if(!entered && mouse->mouseLeaveCallback){
            mouse->mouseLeaveCallback();
        }
    }

    void GlfwMouse::position_changed(GLFWwindow* window, double xpos, double ypos){
        // Get current mouse instance
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        GlfwMouse* mouse = d_window->m_mouse;

        // Check for null
        if(mouse == nullptr){
            // Somehow theres a mouse event for a window with no mouse? Attempt recovery and cleanup
            Logger::println(LogLevel::Critical, "Mouse", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Save position
        mouse->m_position.x = xpos;
        mouse->m_position.y = ypos;

        // Create event for the engine
        if(mouse->mousePosCallback){
            mouse->mousePosCallback(mouse->m_position);
        }
    }

    void GlfwMouse::button_pressed(GLFWwindow* window, int button, int action, int mods){
        // Get current mouse instance
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        GlfwMouse* mouse = d_window->m_mouse;

        // Check for null
        if(mouse == nullptr){
            // Somehow theres a mouse event for a window with no mouse? Attempt recovery and cleanup
            Logger::println(LogLevel::Critical, "Mouse", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        if(action == GLFW_RELEASE){
            mouse->m_lastPressedKeys[button] = false;
        }

        // Convert to draft actions
        if(mouse->mouseButtonCallback){
            mouse->mouseButtonCallback(button, action, mods);
        }
    }

    void GlfwMouse::mouse_scrolled(GLFWwindow* window, double xoffset, double yoffset){
        // Get current mouse instance
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        GlfwMouse* mouse = d_window->m_mouse;

        // Check for null
        if(mouse == nullptr){
            // Somehow theres a mouse event for a window with no mouse? Attempt recovery and cleanup
            Logger::println(LogLevel::Critical, "Mouse", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Set state
        mouse->m_lastScrollDelta.x = xoffset;
        mouse->m_lastScrollDelta.y = yoffset;

        // Handle callback
        if(mouse->mouseScrollCallback){
            mouse->mouseScrollCallback(mouse->m_lastScrollDelta);
        }
    }

    // Constructor
    GlfwMouse::GlfwMouse(Window& window) : m_window(&window) {
        // Install the object into the window if it doesn't already have a mouse
        assert(m_window->m_mouse == nullptr && "Window already has a mouse, only one is supported");
        m_window->m_mouse = this;

        // Install GLFW callbacks
        glfwSetMouseButtonCallback(window.get_glfw_handle(), GlfwMouse::button_pressed);
        glfwSetScrollCallback(window.get_glfw_handle(), GlfwMouse::mouse_scrolled);
        glfwSetCursorPosCallback(window.get_glfw_handle(), GlfwMouse::position_changed);
        glfwSetCursorEnterCallback(window.get_glfw_handle(), GlfwMouse::mouse_entered);
    }

    GlfwMouse::~GlfwMouse(){
        // Uninstall GLFW callbacks and then remove from the window
        if(m_window){
            cleanup_callbacks(m_window->get_glfw_handle());
            m_window->m_mouse = nullptr;
        }
    }

    // Functions
    void GlfwMouse::set_position(const Vector2f& pos){
        glfwSetCursorPos(m_window->get_glfw_handle(), pos.x, pos.y);
        m_position = pos;
    }

    bool GlfwMouse::is_hovered() const {
        return glfwGetWindowAttrib(m_window->get_glfw_handle(), GLFW_HOVERED);
    }

    bool GlfwMouse::is_pressed(int button) const {
        return glfwGetMouseButton(m_window->get_glfw_handle(), button) == GLFW_PRESS;
    }

    bool GlfwMouse::is_just_pressed(int button) const {
        bool result = is_pressed(button);
        bool state = m_lastPressedKeys[button];
        m_lastPressedKeys[button] = result;
        return result && !state;
    }

    const Vector2d& GlfwMouse::get_scroll() const {
        return m_lastScrollDelta;
    }

    const Vector2d& GlfwMouse::get_position() const {
        glfwGetCursorPos(m_window->get_glfw_handle(), &m_position.x, &m_position.y);
        return m_position;
    }

    const Vector2d GlfwMouse::get_normalized_position() const {
        auto size = m_window->get_size();
        UIntRect rect{0, 0, size.x, size.y};
        Vector2d coords = get_position();

        return {
            ((coords.x - rect.x) / rect.width - 0.5) * 2.0,
            (1.0 - (coords.y - rect.y) / rect.height - 0.5) * 2.0
        };
    }

    bool GlfwMouse::is_valid() const {
        return m_window != nullptr;
    }

    /// Fake mouse
    // Constructor
    FakeMouse::FakeMouse(const Vector2u& size){
        // Install the object into the window if it doesn't already have a mouse
        set_window_size(size);
    }

    // Functions
    void FakeMouse::set_position(const Vector2f& pos){
        m_position = pos;
    }

    bool FakeMouse::is_hovered() const {
        return hovered;
    }

    bool FakeMouse::is_pressed(int button) const {
        return m_pressedKeys[button];
    }

    bool FakeMouse::is_just_pressed(int button) const {
        bool result = is_pressed(button);
        bool state = m_lastPressedKeys[button];
        m_lastPressedKeys[button] = result;
        return result && !state;
    }

    const Vector2d& FakeMouse::get_scroll() const {
        return m_lastScrollDelta;
    }

    const Vector2d& FakeMouse::get_position() const {
        return m_position;
    }

    const Vector2d FakeMouse::get_normalized_position() const {
        UIntRect rect{0, 0, m_size.x, m_size.y};
        Vector2d coords = get_position();

        return {
            ((coords.x - rect.x) / rect.width - 0.5) * 2.0,
            (1.0 - (coords.y - rect.y) / rect.height - 0.5) * 2.0
        };
    }

    bool FakeMouse::is_valid() const {
        return true;
    }

    void FakeMouse::set_window_size(const Vector2u& size){
        m_size = size;
    }

    void FakeMouse::mouse_entered(int entered){
        // Create event for the engine
        if(entered && mouseEnterCallback){
            mouseEnterCallback();
            hovered = true;
        } else if(!entered && mouseLeaveCallback){
            mouseLeaveCallback();
            hovered = false;
        }
    }

    void FakeMouse::position_changed(double xpos, double ypos){
        // Save position
        m_position.x = xpos;
        m_position.y = ypos;

        // Create event for the engine
        if(mousePosCallback){
            mousePosCallback(m_position);
        }
    }

    void FakeMouse::button_pressed(int button, int action, int mods){
        if(action == GLFW_RELEASE){
            m_lastPressedKeys[button] = false;
            m_pressedKeys[button] = false;
        } else if(action == GLFW_PRESS) {
            m_pressedKeys[button] = true;
        }

        // Convert to draft actions
        if(mouseButtonCallback){
            mouseButtonCallback(button, action, mods);
        }
    }

    void FakeMouse::mouse_scrolled(double xoffset, double yoffset){
        // Set state
        m_lastScrollDelta.x = xoffset;
        m_lastScrollDelta.y = yoffset;

        // Handle callback
        if(mouseScrollCallback){
            mouseScrollCallback(m_lastScrollDelta);
        }
    }
}
