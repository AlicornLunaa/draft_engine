#include "draft/input/keyboard.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"

#include "imgui.h"

#include <cassert>

namespace Draft {
    // Static functions
    void Keyboard::cleanup_callbacks(GLFWwindow* window){
        glfwSetKeyCallback(window, nullptr);
        glfwSetCharCallback(window, nullptr);
    }

    void Keyboard::key_press(GLFWwindow* window, int key, int scancode, int action, int mods){
        // Get current keyboard instance being affected
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        Keyboard* keyboard = d_window->m_keyboard;

        // Check for null
        if(keyboard == nullptr){
            // Somehow theres a keyboard event for a window with no keyboard? Attempt recovery and cleanup
            Logger::println(Level::CRITICAL, "Keyboard", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Reset keypresses
        if(action == GLFW_RELEASE){
            keyboard->m_lastPressedKeys[key] = false;
        }

        // Execute the callback if it has been set
        if(keyboard->keyCallback){
            keyboard->keyCallback(key, scancode, action);
        }
    }

    void Keyboard::text_entered(GLFWwindow* window, unsigned int codepoint){
        // Get current keyboard instance being affected
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        Keyboard* keyboard = d_window->m_keyboard;

        // Check for null
        if(keyboard == nullptr){
            // Somehow theres a keyboard event for a window with no keyboard? Attempt recovery and cleanup
            Logger::println(Level::CRITICAL, "Keyboard", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Execute the callback if it has been set
        if(keyboard->textCallback){
            keyboard->textCallback(codepoint);
        }
    }

    // Constructors
    Keyboard::Keyboard(RenderWindow& window) : m_window(&window) {
        // Install the object into the window if it doesn't already have a keyboard
        assert(m_window->m_keyboard == nullptr && "Window already has a keyboard, only one is supported");
        m_window->m_keyboard = this;

        // Install GLFW callbacks for handles
        glfwSetKeyCallback(window.get_glfw_handle(), Keyboard::key_press);
        glfwSetCharCallback(window.get_glfw_handle(), Keyboard::text_entered);
    }

    Keyboard::~Keyboard(){
        // Uninstall GLFW callbacks and then remove from the window
        cleanup_callbacks(m_window->get_glfw_handle());

        if(m_window){
            m_window->m_keyboard = nullptr;
        }
    }

    // Functions
    bool Keyboard::is_pressed(int key) const {
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetKey(m_window->get_glfw_handle(), key) == GLFW_PRESS);
        return res && !io.WantCaptureKeyboard;
    }

    bool Keyboard::is_just_pressed(int key) const {
        bool result = is_pressed(key);
        bool state = m_lastPressedKeys[key];
        m_lastPressedKeys[key] = result;
        return result && !state;
    }

    bool Keyboard::is_valid() const {
        return m_window != nullptr;
    }
};