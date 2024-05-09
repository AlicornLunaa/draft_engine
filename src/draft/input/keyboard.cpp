#include "draft/input/keyboard.hpp"
#include "draft/input/event.hpp"
#include "GLFW/glfw3.h"
#include "draft/rendering/render_window.hpp"
#include "draft/util/logger.hpp"
#include "imgui.h"

namespace Draft {
    // Forward declared class
    struct Keyboard::GLFWImpl {
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
            // Key current keyboard instance being affected
            Keyboard* keyboard = Keyboard::windowKeyboardMap[(void*)window];

            // Check for null
            if(keyboard == nullptr){
                Logger::println(Level::CRITICAL, "Keyboard", "Something is wrong!");
                exit(0);
            }

            // Reset keypresses
            if(action == GLFW_RELEASE)
                keyboard->set_key_released(key);

            // Convert to draft actions
            Event event{};
            event.key.code = key;
            event.key.alt = (mods & static_cast<int>(Modifier::ALT));
            event.key.control = (mods & static_cast<int>(Modifier::CTRL));
            event.key.shift = (mods & static_cast<int>(Modifier::SHIFT));
            event.key.system = (mods & static_cast<int>(Modifier::SUPER));

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

            for(auto func : keyboard->callbacks){
                func(event);
            }
        }

        static void text_callback(GLFWwindow* window, unsigned int codepoint){
            // Key current keyboard instance being affected
            Keyboard* keyboard = Keyboard::windowKeyboardMap[(void*)window];

            // Check for null
            if(keyboard == nullptr){
                Logger::println(Level::CRITICAL, "Keyboard", "Something is wrong!");
                exit(0);
            }

            // Create event
            Event event{};
            event.type = Event::TextEntered;
            event.text.unicode = codepoint;
            
            // Run event listeners
            for(auto func : keyboard->callbacks){
                func(event);
            }
        }
    };

    // Static variables
    std::unordered_map<void*, Keyboard*> Keyboard::windowKeyboardMap{};

    // Private functions
    void Keyboard::set_key_released(int key){
        lastPressedKeys[key] = false;
    }

    // Constructors
    Keyboard::Keyboard(RenderWindow& window) : window(&window) {
        glfwSetKeyCallback((GLFWwindow*)window.get_raw_window(), GLFWImpl::key_callback);
        glfwSetCharCallback((GLFWwindow*)window.get_raw_window(), GLFWImpl::text_callback);
        window.init_callbacks();
        Keyboard::windowKeyboardMap[window.get_raw_window()] = this;
    }

    Keyboard::~Keyboard(){
        glfwSetKeyCallback((GLFWwindow*)window->get_raw_window(), nullptr);
        glfwSetCharCallback((GLFWwindow*)window->get_raw_window(), nullptr);
        Keyboard::windowKeyboardMap[window->get_raw_window()] = nullptr;
    }

    // Functions
    void Keyboard::add_callback(EventCallback func){
        callbacks.push_back(func);
    }

    void Keyboard::clear_callbacks(){
        callbacks.clear();
    }

    bool Keyboard::is_pressed(int key) const {
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetKey((GLFWwindow*)window->get_raw_window(), key) == GLFW_PRESS);
        lastPressedKeys[key] = res;
        return res && !io.WantCaptureKeyboard;
    }

    bool Keyboard::is_just_pressed(int key) const {
        ImGuiIO& io = ImGui::GetIO();
        bool res = (glfwGetKey((GLFWwindow*)window->get_raw_window(), key) == GLFW_PRESS);
        bool oldState = lastPressedKeys[key];
        lastPressedKeys[key] = res;
        return res && !oldState && !io.WantCaptureKeyboard;;
    }
};