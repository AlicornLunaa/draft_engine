#include "draft/input/mouse.hpp"
#include "GLFW/glfw3.h"
#include "imgui.h"

namespace Draft {
    // Variables
    RenderWindow* Mouse::window = nullptr;
    std::unordered_map<int, bool> Mouse::lastPressedKeys{};
    Vector2d Mouse::position{};

    // Raw function
    void mouse_callback(GLFWwindow* window, int button, int action, int mods){
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);

        if(action == GLFW_RELEASE){
            Mouse::set_button_released(button);
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