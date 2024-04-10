#include "draft/input/mouse.hpp"
#include "GLFW/glfw3.h"
#include "draft/input/event.hpp"
#include "imgui.h"

namespace Draft {
    // Variables
    RenderWindow* Mouse::window = nullptr;
    std::unordered_map<int, bool> Mouse::lastPressedKeys{};
    std::vector<EventCallback> Mouse::callbacks{};
    Vector2d Mouse::position{};

    // Raw function
    void mouse_callback(GLFWwindow* window, int button, int action, int mods){
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);

        if(action == GLFW_RELEASE){
            Mouse::set_button_released(button);
        }

        // Convert to draft actions
        auto pos = Mouse::get_position();
        Event event{};
        event.mouseButton.button = button;
        event.mouseButton.x = pos.x;
        event.mouseButton.y = pos.y;

        switch(action){
        case GLFW_PRESS:
            event.type = Event::MouseButtonPressed;
            break;

        case GLFW_RELEASE:
            event.type = Event::MouseButtonReleased;
            break;

        default:
            break;
        }

        for(auto func : Mouse::callbacks){
            // RenderWindow& window, int key, Action action, int mods
            func(event);
        }
    }

    // Functions
    void Mouse::init(RenderWindow* window){
        cleanup();
        Mouse::window = window;
        glfwSetMouseButtonCallback((GLFWwindow*)window->get_raw_window(), mouse_callback);
    }

    void Mouse::cleanup(){
        if(!window) return; // Avoid cleaning up nothing
        glfwSetMouseButtonCallback((GLFWwindow*)window->get_raw_window(), nullptr);
        Mouse::window = nullptr;
    }

    void Mouse::set_button_released(int button){
        lastPressedKeys[button] = false;
    }

    void Mouse::add_callback(EventCallback func){
        callbacks.push_back(func);
    }

    void Mouse::clear_callbacks(){
        callbacks.clear();
    }

    bool Mouse::is_pressed(int button){
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetMouseButton((GLFWwindow*)window->get_raw_window(), button) == GLFW_PRESS);
        lastPressedKeys[button] = res;
        return res && !io.WantCaptureMouse;
    }

    bool Mouse::is_just_pressed(int button){
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetMouseButton((GLFWwindow*)window->get_raw_window(), button) == GLFW_PRESS);
        bool oldState = lastPressedKeys[button];
        lastPressedKeys[button] = res;
        return res && !oldState && !io.WantCaptureMouse;
    }

    const Vector2d& Mouse::get_position(){
        glfwGetCursorPos((GLFWwindow*)window->get_raw_window(), &position.x, &position.y);
        return position;
    }

    void Mouse::set_position(const Vector2f& pos){
        glfwSetCursorPos((GLFWwindow*)window->get_raw_window(), pos.x, pos.y);
        position.set(pos.x, pos.y);
    }
};