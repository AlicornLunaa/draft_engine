#include <string>

#include "draft/core/application.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/core/scene.hpp"

#include "tracy/Tracy.hpp"

#define GLFW_INCLUDE_NONE

namespace Draft {
    // Constructors
    Application::Application(const char* title, const unsigned int width, const unsigned int height) : window(width, height, title), keyboard(window), mouse(window) {
        // Register callback
        keyboard.add_callback([this](Event e){ window.queue_event(e); });
        mouse.add_callback([this](Event e){ window.queue_event(e); });

        // Register basic commands
        console.register_cmd("cl_vsync", [this](ConsoleArgs args){
            window.set_vsync(std::stoi(args[1]) > 0);
            return true;
        });
    }

    Application::~Application(){
    }

    // Functions
    void Application::run(){
        // Start application loop
        while(window.is_open()){
            // Clock reset
            deltaTime = deltaClock.restart();

            // Handle control events
            while(window.poll_events(event)){
                switch(event.type){
                case Event::Closed:
                    window.close();
                    break;

                default:
                    if(activeScene)
                        activeScene->handle_event(event);
                    break;
                }
            }

            // Fixed physics time-step
            accumulator += deltaTime.as_seconds();

            while(accumulator >= timeStep.as_seconds()){
                if(activeScene)
                    activeScene->update(timeStep);

                accumulator -= timeStep.as_seconds();
            }
            
            // Rendering stuff!
            window.clear();

            if(activeScene)
                activeScene->render(deltaTime);

            // Draw debug stuff
            stats.draw(*this);
            console.draw();
            
            window.display();

            // Profiler setup
            FrameMarkNamed("main");
        }
    }

    void Application::reset_timers(){
        accumulator = 0.f;
        deltaClock.restart();
        deltaTime = Time();
    }

    void Application::set_scene(Scene* scene){
        if(activeScene)
            // Detach event on previous scene
            activeScene->on_detach();

        reset_timers(); // Reset dt to avoid large jumps in physics
        activeScene = scene;

        if(activeScene)
            // Attach event on new scene
            activeScene->on_attach();
    }

    Scene* Application::get_scene() const {
        return activeScene;
    }
}