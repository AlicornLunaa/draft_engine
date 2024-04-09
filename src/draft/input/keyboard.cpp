#include "draft/input/keyboard.hpp"
#include "GLFW/glfw3.h"

namespace Draft {
    // Variables
    RenderWindow* Keyboard::window = nullptr;
    std::unordered_map<int, bool> Keyboard::lastPressedKeys{};

    // Raw function
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
        if(action == GLFW_RELEASE){
            Keyboard::set_key_released(key);
        }
    }

    // Functions
    void Keyboard::init(RenderWindow* window){
        cleanup();
        Keyboard::window = window;
        glfwSetKeyCallback((GLFWwindow*)window->get_raw_window(), key_callback);
    }

    void Keyboard::cleanup(){
        if(!window) return; // Avoid cleaning up nothing
        glfwSetKeyCallback((GLFWwindow*)window->get_raw_window(), nullptr);
        Keyboard::window = nullptr;
    }

    void Keyboard::set_key_released(int key){
        lastPressedKeys[key] = false;
    }

    bool Keyboard::is_pressed(int key){
        bool res = (glfwGetKey((GLFWwindow*)window->get_raw_window(), key) == GLFW_PRESS);
        lastPressedKeys[key] = res;
        return res;
    }

    bool Keyboard::is_just_pressed(int key){
        bool res = (glfwGetKey((GLFWwindow*)window->get_raw_window(), key) == GLFW_PRESS);
        bool oldState = lastPressedKeys[key];
        lastPressedKeys[key] = res;
        return res && !oldState;
    }
};