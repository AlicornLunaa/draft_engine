#include "draft/math/rect.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/input/mouse.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"
#include "imgui.h"

namespace Draft {
    // Static functions
    void Mouse::cleanup_callbacks(GLFWwindow* window){
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetScrollCallback(window, nullptr);
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetCursorEnterCallback(window, nullptr);
    }

    void Mouse::mouse_entered(GLFWwindow* window, int entered){
        // Get current mouse instance
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        Mouse* mouse = d_window->m_mouse;

        // Check for null
        if(mouse == nullptr){
            // Somehow theres a keyboard event for a window with no keyboard? Attempt recovery and cleanup
            Logger::println(Level::CRITICAL, "Mouse", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Skip event if hovered over an IMGUI page
        ImGuiIO& io = ImGui::GetIO();
        
        if(io.WantCaptureMouse)
            return;

        // Create event for the engine
        if(entered && mouse->mouseEnterCallback){
            mouse->mouseEnterCallback();
        } else if(!entered && mouse->mouseLeaveCallback){
            mouse->mouseLeaveCallback();
        }
    }

    void Mouse::position_changed(GLFWwindow* window, double xpos, double ypos){
        // Get current mouse instance
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        Mouse* mouse = d_window->m_mouse;

        // Check for null
        if(mouse == nullptr){
            // Somehow theres a keyboard event for a window with no keyboard? Attempt recovery and cleanup
            Logger::println(Level::CRITICAL, "Mouse", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Skip event if hovered over an IMGUI page
        ImGuiIO& io = ImGui::GetIO();
        
        if(io.WantCaptureMouse)
            return;

        // Save position
        mouse->m_position.x = xpos;
        mouse->m_position.y = ypos;

        // Create event for the engine
        if(mouse->mousePosCallback){
            mouse->mousePosCallback(mouse->m_position);
        }
    }

    void Mouse::button_pressed(GLFWwindow* window, int button, int action, int mods){
        // Get current mouse instance
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        Mouse* mouse = d_window->m_mouse;

        // Check for null
        if(mouse == nullptr){
            // Somehow theres a keyboard event for a window with no keyboard? Attempt recovery and cleanup
            Logger::println(Level::CRITICAL, "Mouse", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);

        if(action == GLFW_RELEASE){
            mouse->m_lastPressedKeys[button] = false;
        }

        // Skip event if hovered over an IMGUI page
        if(io.WantCaptureMouse)
            return;

        // Convert to draft actions
        if(mouse->mouseButtonCallback){
            mouse->mouseButtonCallback(button, action, mods);
        }
    }

    void Mouse::mouse_scrolled(GLFWwindow* window, double xoffset, double yoffset){
        // Get current mouse instance
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        Mouse* mouse = d_window->m_mouse;

        // Check for null
        if(mouse == nullptr){
            // Somehow theres a keyboard event for a window with no keyboard? Attempt recovery and cleanup
            Logger::println(Level::CRITICAL, "Mouse", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Skip event if hovered over an IMGUI page
        ImGuiIO& io = ImGui::GetIO();
        
        if(io.WantCaptureMouse)
            return;

        // Set state
        mouse->m_lastScrollDelta.x = xoffset;
        mouse->m_lastScrollDelta.y = yoffset;
        
        // Handle callback
        if(mouse->mouseScrollCallback){
            mouse->mouseScrollCallback(mouse->m_lastScrollDelta);
        }
    }

    // Constructors
    Mouse::Mouse(RenderWindow& window) : m_window(&window) {
        // Install the object into the window if it doesn't already have a keyboard
        assert(m_window->m_mouse == nullptr && "Window already has a mouse, only one is supported");
        m_window->m_mouse = this;

        // Install GLFW callbacks
        glfwSetMouseButtonCallback(window.get_glfw_handle(), Mouse::button_pressed);
        glfwSetScrollCallback(window.get_glfw_handle(), Mouse::mouse_scrolled);
        glfwSetCursorPosCallback(window.get_glfw_handle(), Mouse::position_changed);
        glfwSetCursorEnterCallback(window.get_glfw_handle(), Mouse::mouse_entered);
    }

    Mouse::~Mouse(){
        // Uninstall GLFW callbacks and then remove from the window
        cleanup_callbacks(m_window->get_glfw_handle());

        if(m_window){
            m_window->m_mouse = nullptr;
        }
    }

    // Functions
    void Mouse::set_position(const Vector2f& pos){
        glfwSetCursorPos(m_window->get_glfw_handle(), pos.x, pos.y);
        m_position = pos;
    }

    bool Mouse::is_hovered() const {
        return glfwGetWindowAttrib(m_window->get_glfw_handle(), GLFW_HOVERED);
    }

    bool Mouse::is_pressed(int button) const {
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetMouseButton(m_window->get_glfw_handle(), button) == GLFW_PRESS);
        return res && !io.WantCaptureMouse;
    }

    bool Mouse::is_just_pressed(int button) const {
        bool result = is_pressed(button);
        bool state = m_lastPressedKeys[button];
        m_lastPressedKeys[button] = result;
        return result && !state;
    }

    const Vector2d& Mouse::get_scroll() const {
        return m_lastScrollDelta;
    }

    const Vector2d& Mouse::get_position() const {
        glfwGetCursorPos(m_window->get_glfw_handle(), &m_position.x, &m_position.y);
        return m_position;
    }

    const Vector2d Mouse::get_normalized_position() const {
        return Math::normalize_coordinates(*m_window, get_position());
    }
};