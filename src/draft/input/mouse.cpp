#include "draft/math/rect.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/input/mouse.hpp"
#include "draft/input/event.hpp"
#include "draft/input/keyboard.hpp"

#include "GLFW/glfw3.h"
#include "imgui.h"

namespace Draft {
    // Forward declaration
    struct Mouse::GLFWImpl {
        static void mouse_enter_callback(GLFWwindow* window, int entered){
            Mouse* mouse = Mouse::windowMouseMap[(void*)window];

            // Skip event if hovered over an IMGUI page
            ImGuiIO& io = ImGui::GetIO();
            
            if(io.WantCaptureMouse)
                return;

            // Create event for the engine
            Event event{};
            event.type = (entered ? Event::MouseEntered : Event::MouseLeft);

            for(auto func : mouse->callbacks){
                func(event);
            }
        }

        static void mouse_move_callback(GLFWwindow* window, double x, double y){
            Mouse* mouse = Mouse::windowMouseMap[(void*)window];

            // Skip event if hovered over an IMGUI page
            ImGuiIO& io = ImGui::GetIO();
            
            if(io.WantCaptureMouse)
                return;

            // Create event for the engine
            Event event{};
            event.type = Event::MouseMoved;
            event.mouseMove.x = x;
            event.mouseMove.y = y;

            for(auto func : mouse->callbacks){
                func(event);
            }
        }

        static void mouse_click_callback(GLFWwindow* window, int button, int action, int mods){
            Mouse* mouse = Mouse::windowMouseMap[(void*)window];

            ImGuiIO& io = ImGui::GetIO();
            io.AddMouseButtonEvent(button, action == GLFW_PRESS);

            if(action == GLFW_RELEASE){
                mouse->lastPressedKeys[button] = false;
            }

            // Skip event if hovered over an IMGUI page
            if(io.WantCaptureMouse)
                return;

            // Convert to draft actions
            auto pos = mouse->get_position();
            Event event{};
            event.mouseButton.button = button;
            event.mouseButton.x = pos.x;
            event.mouseButton.y = pos.y;

            switch(action){
            case GLFW_PRESS:
                mouse->lastPressedKeys[button] = true;
                event.type = Event::MouseButtonPressed;
                break;

            case GLFW_RELEASE:
                event.type = Event::MouseButtonReleased;
                break;

            default:
                break;
            }

            for(auto func : mouse->callbacks){
                func(event);
            }
        }

        static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
            Mouse* mouse = Mouse::windowMouseMap[(void*)window];

            // Skip event if hovered over an IMGUI page
            ImGuiIO& io = ImGui::GetIO();
            
            if(io.WantCaptureMouse)
                return;

            // Set state
            mouse->lastScrollDelta.x = xoffset;
            mouse->lastScrollDelta.y = yoffset;

            // Create event
            Event event{};
            event.type = Event::MouseWheelScrolled;
            event.mouseWheelScroll.x = xoffset;
            event.mouseWheelScroll.y = yoffset;

            for(auto func : mouse->callbacks){
                func(event);
            }
        }
    };

    // Static variables
    std::unordered_map<void*, Mouse*> Mouse::windowMouseMap{};

    // Constructors
    Mouse::Mouse(RenderWindow& window) : window(&window) {
        glfwSetCursorEnterCallback((GLFWwindow*)window.get_raw_window(), GLFWImpl::mouse_enter_callback);
        glfwSetCursorPosCallback((GLFWwindow*)window.get_raw_window(), GLFWImpl::mouse_move_callback);
        glfwSetMouseButtonCallback((GLFWwindow*)window.get_raw_window(), GLFWImpl::mouse_click_callback);
        glfwSetScrollCallback((GLFWwindow*)window.get_raw_window(), GLFWImpl::mouse_scroll_callback);
        Mouse::windowMouseMap[window.get_raw_window()] = this;
    }

    Mouse::~Mouse(){
        glfwSetCursorEnterCallback((GLFWwindow*)window->get_raw_window(), nullptr);
        glfwSetCursorPosCallback((GLFWwindow*)window->get_raw_window(), nullptr);
        glfwSetMouseButtonCallback((GLFWwindow*)window->get_raw_window(), nullptr);
        glfwSetScrollCallback((GLFWwindow*)window->get_raw_window(), nullptr);
        Mouse::windowMouseMap[window->get_raw_window()] = nullptr;
    }

    // Functions
    void Mouse::add_callback(EventCallback func){
        callbacks.push_back(func);
    }

    void Mouse::clear_callbacks(){
        callbacks.clear();
    }

    bool Mouse::is_hovered() const {
        return glfwGetWindowAttrib((GLFWwindow*)window->get_raw_window(), GLFW_HOVERED);
    }

    bool Mouse::is_pressed(int button) const {
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetMouseButton((GLFWwindow*)window->get_raw_window(), button) == GLFW_PRESS);
        return res && !io.WantCaptureMouse;
    }

    bool Mouse::is_just_pressed(int button) const {
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetMouseButton((GLFWwindow*)window->get_raw_window(), button) == GLFW_PRESS);
        return res && lastPressedKeys[button] && !io.WantCaptureMouse;
    }

    const Vector2d& Mouse::get_scroll() const {
        return lastScrollDelta;
    }

    const Vector2d& Mouse::get_position() const {
        glfwGetCursorPos((GLFWwindow*)window->get_raw_window(), &position.x, &position.y);
        return position;
    }

    const Vector2d Mouse::get_normalized_position() const {
        return Math::normalize_coordinates(*window, get_position());
    }

    void Mouse::set_position(const Vector2f& pos){
        glfwSetCursorPos((GLFWwindow*)window->get_raw_window(), pos.x, pos.y);
        position = { pos.x, pos.y };
    }

    void Mouse::reset_mouse_state(){
        lastScrollDelta = {};
        lastPressedKeys.clear();
    }
};