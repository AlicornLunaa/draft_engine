#include <algorithm>
#include <functional>
#include <string>

#include "draft/core/application.hpp"
#include "draft/core/scene.hpp"

#include "draft/input/action.hpp"
#include "tracy/Tracy.hpp"

namespace Draft {
    // Private functions
    void Application::framebuffer_resized(uint width, uint height){
        event.type = Event::Resized;
        event.size.width = width;
        event.size.height = height;

        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::window_focus(bool focused){
        event.type = focused ? Event::GainedFocus : Event::LostFocus;

        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::window_closed(){
        event.type = Event::Closed;

        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::key_callback(int key, int action, int modifier){
        // Set event type
        if(action == Action::PRESS) event.type = Event::KeyPressed;
        else if(action == Action::RELEASE) event.type = Event::KeyReleased;
        else if(action == Action::HOLD) event.type = Event::KeyHold;

        // Set key data
        event.key.code = key;
        event.key.alt = modifier & static_cast<int>(Keyboard::Modifier::ALT);
        event.key.control = modifier & static_cast<int>(Keyboard::Modifier::CTRL);
        event.key.shift = modifier & static_cast<int>(Keyboard::Modifier::SHIFT);
        event.key.system = modifier & static_cast<int>(Keyboard::Modifier::SUPER);
        event.key.mods = modifier;

        // Send event along
        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::text_callback(unsigned int codepoint){
        // Set event type
        event.type = Event::TextEntered;
        event.text.unicode = codepoint;

        // Send event along
        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::mouse_button_callback(int button, int action, int modifier){
        // Set event type
        if(action == Action::PRESS) event.type = Event::MouseButtonPressed;
        else if(action == Action::RELEASE) event.type = Event::MouseButtonReleased;

        // Get data for event
        const Vector2d& v = mouse.get_position();

        // Set key data
        event.mouseButton.button = button;
        event.mouseButton.x = v.x;
        event.mouseButton.y = v.y;
        event.mouseButton.mods = modifier;

        // Send event along
        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::mouse_position_callback(const Vector2d& position){
        // Set event type
        event.type = Event::MouseMoved;
        event.mouseMove.x = position.x;
        event.mouseMove.y = position.y;

        // Send event along
        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::mouse_scroll_callback(const Vector2d& delta){
        // Set event type
        event.type = Event::MouseWheelScrolled;
        event.mouseWheelScroll.x = delta.x;
        event.mouseWheelScroll.y = delta.y;

        // Send event along
        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::mouse_enter_callback(){
        event.type = Event::MouseEntered;

        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::mouse_leave_callback(){
        event.type = Event::MouseLeft;

        if(activeScene)
            activeScene->handle_event(event);
    }

    void Application::tick(){
        // This function does a fixed time-step update
        ZoneScopedNCS("fixed_tick", 0xff3333, 20);

        accumulator += deltaTime.as_seconds();
        accumulator = std::min(accumulator, (double)maxAccumulator.as_seconds()); // Needed to prevent accumulator spiral

        while(accumulator >= timeStep.as_seconds()){
            if(activeScene)
                activeScene->update(timeStep);

            accumulator -= timeStep.as_seconds();
        }
    }

    void Application::frame(){
        // This function does a single frame in the application
        ZoneScopedNCS("frame_tick", 0x33ff00, 20);
        
        window.clear();
        imgui.start_frame();

        if(activeScene)
            activeScene->render(deltaTime);

        // Draw debug stuff
        stats.draw(*this);
        console.draw();
        
        imgui.end_frame();
        window.display();
    }

    // Constructors
    Application::Application(const char* title, const unsigned int width, const unsigned int height) : window(width, height, title), keyboard(window), mouse(window) {
        // Start the profiler
        ZoneScopedN("app_creation");

        // Register callbacks
        window.frameSizeCallback = std::bind(&Application::framebuffer_resized, this, std::placeholders::_1, std::placeholders::_2);
        window.focusCallback = std::bind(&Application::window_focus, this, std::placeholders::_1);
        window.closeCallback = std::bind(&Application::window_closed, this);
        keyboard.keyCallback = std::bind(&Application::key_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        keyboard.textCallback = std::bind(&Application::text_callback, this, std::placeholders::_1);
        mouse.mouseButtonCallback = std::bind(&Application::mouse_button_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        mouse.mousePosCallback = std::bind(&Application::mouse_position_callback, this, std::placeholders::_1);
        mouse.mouseScrollCallback = std::bind(&Application::mouse_scroll_callback, this, std::placeholders::_1);
        mouse.mouseEnterCallback = std::bind(&Application::mouse_enter_callback, this);
        mouse.mouseLeaveCallback = std::bind(&Application::mouse_leave_callback, this);

        // Register basic commands
        console.register_cmd("cl_vsync", [this](ConsoleArgs args){
            window.set_vsync(std::stoi(args[1]) > 0);
            return true;
        });
    }

    Application::~Application(){
        ZoneScopedN("app_destruction");
    }

    // Functions
    void Application::run(){
        // Start application loop
        while(window.is_open()){
            // Clock reset
            deltaTime = deltaClock.restart();

            // Handle control events
            window.poll_events();

            // Fixed physics time-step scope
            tick();
            
            // Rendering stuff!
            frame();

            // Profiler setup
            TracyPlot("frame time", deltaTime.as_seconds());
            FrameMark;
        }
    }

    void Application::reset_timers(){
        accumulator = 0.f;
        deltaClock.restart();
        deltaTime = Time();
    }

    void Application::set_scene(Scene* scene){
        ZoneScopedN("scene_change");

        if(activeScene)
            // Detach event on previous scene
            activeScene->on_detach();

        reset_timers(); // Reset dt to avoid large jumps in physics
        
        if(scene)
            // Attach event on new scene
            scene->on_attach();

        activeScene = scene;
    }

    Scene* Application::get_scene() const {
        return activeScene;
    }
}