#include "draft/input/keyboard.hpp"
#include "draft/input/event.hpp"
#include "draft/input/keys.hpp"
#include "GLFW/glfw3.h"
#include "imgui.h"

namespace Draft {
    // Variables
    RenderWindow* Keyboard::window = nullptr;
    std::unordered_map<int, bool> Keyboard::lastPressedKeys{};
    std::vector<EventCallback> Keyboard::callbacks{};

    // Raw function
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
        if(action == GLFW_RELEASE){
            Keyboard::set_key_released(key);
        }

        // Convert to draft actions
        Event event{};
        event.key.code = key;
        event.key.alt = (mods & KeyModifier::ALT);
        event.key.control = (mods & KeyModifier::CTRL);
        event.key.shift = (mods & KeyModifier::SHIFT);
        event.key.system = (mods & KeyModifier::SUPER);

        switch(action){
        case GLFW_PRESS:
            event.type = Event::KeyPressed;
            break;

        case GLFW_RELEASE:
            event.type = Event::KeyReleased;
            break;

        case GLFW_REPEAT:
            event.type = Event::KeyHold;
            break;

        default:
            break;
        }

        for(auto func : Keyboard::callbacks){
            // RenderWindow& window, int key, Action action, int mods
            func(event);
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

    void Keyboard::add_callback(EventCallback func){
        callbacks.push_back(func);
    }

    void Keyboard::clear_callbacks(){
        callbacks.clear();
    }

    bool Keyboard::is_pressed(int key){
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetKey((GLFWwindow*)window->get_raw_window(), key) == GLFW_PRESS);
        lastPressedKeys[key] = res;
        return res && !io.WantCaptureKeyboard;
    }

    bool Keyboard::is_just_pressed(int key){
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetKey((GLFWwindow*)window->get_raw_window(), key) == GLFW_PRESS);
        bool oldState = lastPressedKeys[key];
        lastPressedKeys[key] = res;
        return res && !oldState && !io.WantCaptureKeyboard;;
    }
};