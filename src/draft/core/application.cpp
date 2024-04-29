#define GLFW_INCLUDE_NONE

#include <string>
#include <iostream>

#include "draft/core/application.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/core/scene.hpp"
#include "draft/widgets/stats.hpp"

namespace Draft {
    // Constructors
    Application::Application(const char* title, const unsigned int width, const unsigned int height) : window(width, height, title), keyboard(window), mouse(window) {
        // Redirect cout to console
        oldOutBuf = std::cout.rdbuf(console.get_stream().rdbuf());

        // Register callback
        keyboard.add_callback([this](Event e){ window.queue_event(e); });
        mouse.add_callback([this](Event e){ window.queue_event(e); });

        // Register basic commands
        console.register_cmd("reload_assets", [this](ConsoleArgs args){
            assets.reload();
            return true;
        });

        console.register_cmd("cl_vsync", [this](ConsoleArgs args){
            window.set_vsync(std::stoi(args[1]) > 0);
            return true;
        });
    }

    Application::~Application(){
        // Restore cout to stdout
        std::cout.rdbuf(oldOutBuf);
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
                        activeScene->handleEvent(event);
                    break;
                }
            }

            // Fixed physics time-step
            accumulator += deltaTime.as_seconds();

            while(accumulator >= timeStep){
                if(activeScene)
                    activeScene->update(deltaTime);

                accumulator -= timeStep;
            }
            
            // Rendering stuff!
            window.clear();

            if(activeScene)
                activeScene->render(deltaTime);

            // Draw debug stuff
            stats.draw(*this);
            console.draw();
            
            window.display();
        }
    }
}