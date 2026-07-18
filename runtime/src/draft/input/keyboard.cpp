#include "draft/input/keyboard.hpp"
#include "draft/util/logger.hpp"
#include "draft/rendering/window.hpp"

#include "GLFW/glfw3.h"

#include <cassert>

namespace Draft {
    /// GlfwKeyboard
    // Static functions
    void GlfwKeyboard::cleanup_callbacks(GLFWwindow* window){
        glfwSetKeyCallback(window, nullptr);
        glfwSetCharCallback(window, nullptr);
    }

    void GlfwKeyboard::key_press(GLFWwindow* window, int key, int scancode, int action, int mods){
        (void)scancode;

        // Get current keyboard instance being affected
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        GlfwKeyboard* keyboard = d_window->m_keyboard;

        // Check for null
        if(keyboard == nullptr){
            // Somehow theres a keyboard event for a window with no keyboard? Attempt recovery and cleanup
            Logger::println(LogLevel::Critical, "Keyboard", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Reset keypresses
        if(action == GLFW_RELEASE){
            keyboard->m_lastPressedKeys[key] = false;
        }

        // Execute the callback if it has been set
        if(keyboard->keyCallback){
            keyboard->keyCallback(key, action, mods);
        }
    }

    void GlfwKeyboard::text_entered(GLFWwindow* window, unsigned int codepoint){
        // Get current keyboard instance being affected
        Window* d_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        GlfwKeyboard* keyboard = d_window->m_keyboard;

        // Check for null
        if(keyboard == nullptr){
            // Somehow theres a keyboard event for a window with no keyboard? Attempt recovery and cleanup
            Logger::println(LogLevel::Critical, "Keyboard", "Something is wrong!");
            cleanup_callbacks(d_window->get_glfw_handle());
            return;
        }

        // Execute the callback if it has been set
        if(keyboard->textCallback){
            keyboard->textCallback(codepoint);
        }
    }

    // Constructor
    GlfwKeyboard::GlfwKeyboard(Window& window) : m_window(&window) {
        // Install the object into the window if it doesn't already have a keyboard
        assert(m_window->m_keyboard == nullptr && "Window already has a keyboard, only one is supported");
        m_window->m_keyboard = this;

        // Install GLFW callbacks for handles
        glfwSetInputMode(window.get_glfw_handle(), GLFW_LOCK_KEY_MODS, GLFW_TRUE);
        glfwSetKeyCallback(window.get_glfw_handle(), GlfwKeyboard::key_press);
        glfwSetCharCallback(window.get_glfw_handle(), GlfwKeyboard::text_entered);
    }

    GlfwKeyboard::~GlfwKeyboard(){
        // Uninstall GLFW callbacks and then remove from the window
        if(m_window){
            cleanup_callbacks(m_window->get_glfw_handle());
            m_window->m_keyboard = nullptr;
        }
    }

    // Functions
    bool GlfwKeyboard::is_pressed(int key) const {
        return glfwGetKey(m_window->get_glfw_handle(), key) == GLFW_PRESS;
    }

    bool GlfwKeyboard::is_just_pressed(int key) const {
        bool result = is_pressed(key);
        bool state = m_lastPressedKeys[key];
        m_lastPressedKeys[key] = result;
        return result && !state;
    }

    int GlfwKeyboard::get_modifiers() const {
        // Get state and return it
        int state = 0;
        state |= ((is_pressed(Keyboard::LEFT_SHIFT) | is_pressed(Keyboard::RIGHT_SHIFT)) ? static_cast<int>(Modifier::SHIFT) : 0);
        state |= ((is_pressed(Keyboard::LEFT_CONTROL) | is_pressed(Keyboard::RIGHT_CONTROL)) ? static_cast<int>(Modifier::CTRL) : 0);
        state |= ((is_pressed(Keyboard::LEFT_ALT) | is_pressed(Keyboard::RIGHT_ALT)) ? static_cast<int>(Modifier::ALT) : 0);
        state |= ((is_pressed(Keyboard::LEFT_SUPER) | is_pressed(Keyboard::RIGHT_SUPER)) ? static_cast<int>(Modifier::SUPER) : 0);
        return state;
    }

    bool GlfwKeyboard::is_valid() const {
        return m_window != nullptr;
    }

    /// Fake keyboard
    // Functions
    bool FakeKeyboard::is_pressed(int key) const {
        return m_pressedKeys[key];
    }

    bool FakeKeyboard::is_just_pressed(int key) const {
        bool result = is_pressed(key);
        bool state = m_lastPressedKeys[key];
        m_lastPressedKeys[key] = result;
        return result && !state;
    }

    int FakeKeyboard::get_modifiers() const {
        // Get state and return it
        int state = 0;
        state |= ((is_pressed(Keyboard::LEFT_SHIFT) | is_pressed(Keyboard::RIGHT_SHIFT)) ? static_cast<int>(Modifier::SHIFT) : 0);
        state |= ((is_pressed(Keyboard::LEFT_CONTROL) | is_pressed(Keyboard::RIGHT_CONTROL)) ? static_cast<int>(Modifier::CTRL) : 0);
        state |= ((is_pressed(Keyboard::LEFT_ALT) | is_pressed(Keyboard::RIGHT_ALT)) ? static_cast<int>(Modifier::ALT) : 0);
        state |= ((is_pressed(Keyboard::LEFT_SUPER) | is_pressed(Keyboard::RIGHT_SUPER)) ? static_cast<int>(Modifier::SUPER) : 0);
        return state;
    }

    bool FakeKeyboard::is_valid() const {
        return true;
    }

    void FakeKeyboard::key_press(int key, int action, int mods){
        if(action == GLFW_RELEASE){
            m_lastPressedKeys[key] = false;
            m_pressedKeys[key] = false;
        } else if(action == GLFW_PRESS){
            m_pressedKeys[key] = true;
        }

        // Execute the callback if it has been set
        if(keyCallback){
            keyCallback(key, action, mods);
        }
    }

    void FakeKeyboard::text_entered(unsigned int codepoint){
        // Execute the callback if it has been set
        if(textCallback){
            textCallback(codepoint);
        }
    }
}
