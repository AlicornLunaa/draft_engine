#define GLFW_INCLUDE_NONE

#include <string>
#include <iostream>

#include "draft/core/application.hpp"
#include "draft/core/scene.hpp"
#include "draft/util/logger.hpp"
#include "draft/widgets/stats.hpp"

namespace Draft {
    // Constructors
    Application::Application(const char* title, const unsigned int width, const unsigned int height) : window(width, height, title) {
        // Feedback
        Logger::println(Level::INFO, "Draft Engine", "Initializing...");

        // Register things to load
        assetManager.queue_shader("./assets/shaders/default");
        assetManager.queue_shader("./assets/shaders/shapes");
        assetManager.queue_shader("./assets/shaders/test");
        assetManager.queue_shader("./assets/shaders/mesh");
        assetManager.queue_texture("./assets/textures/test_image_1.png");
        assetManager.queue_texture("./assets/textures/test_image_2.png");
        assetManager.queue_texture("./assets/textures/test_image_3.png");
        assetManager.load();

        // Redirect cout to console
        oldOutBuf = std::cout.rdbuf(console.get_stream().rdbuf());

        // Register basic commands
        console.register_cmd("reload_assets", [this](ConsoleArgs args){
            assetManager.reload();
            return true;
        });

        console.register_cmd("cl_vsync", [this](ConsoleArgs args){
            window.set_vsync(std::stoi(args[1]) > 0);
            return true;
        });
    }

    Application::~Application(){
        // Cleanup
        Logger::println(Level::INFO, "Draft Engine", "Exitting...");

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

            // Fixed time-step
            accumulator += deltaTime.as_seconds();

            while(accumulator >= timeStep){
                if(activeScene){
                    activeScene->update(deltaTime);
                }

                accumulator -= timeStep;
            }
            
            // Rendering stuff!
            window.clear();

            if(activeScene)
                activeScene->render(deltaTime);

            // Draw debug stuff
            if(debug){
                stats.draw(*this);
            }

            console.draw();
            window.display();
        }
    }
}