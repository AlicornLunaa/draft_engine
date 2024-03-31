#include <iostream>

#include "draft/core/application.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    Application::Application(const char* title, const unsigned int width, const unsigned int height)
            : window(sf::RenderWindow(sf::VideoMode(width, height), title)), width(width), height(height) {
        // Feedback
        Logger::println(Level::INFO, "Draft Engine", "Initializing...");
        
        // Make sure game can run, handle arguments, etc
        if(!sf::Shader::isAvailable()){
            Logger::println(Level::CRITICAL, "Draft Engine", "Shaders unavailable, maybe OpenGL is too old? Cannot continue.");
            exit(1);
        }

        // ImGUI
        if(!ImGui::SFML::Init(window)){
            Logger::println(Level::CRITICAL, "ImGUI", "Failed to initialize, exitting.");
            exit(1);
        }
        auto& imGuiIO = ImGui::GetIO();
        imGuiIO.IniFilename = nullptr;
        imGuiIO.LogFilename = nullptr;

        // Register basic commands
        console.registerCmd("reload_assets", [this](ConsoleArgs args){
            assetManager.reload();
            return true;
        });

        // Redirect cout to console
        #ifndef DEBUG
        oldOutBuf = std::cout.rdbuf(console.getStream().rdbuf());
        #endif
    }

    Application::~Application(){
        // Cleanup
        Logger::println(Level::INFO, "Draft Engine", "Exitting...");
        ImGui::SFML::Shutdown();

        // Restore cout to stdout
        #ifndef DEBUG
        std::cout.rdbuf(oldOutBuf);
        #endif
    }

    void Application::run(){
        // Start application loop
        while(window.isOpen()){
            // Clock reset
            deltaTime = deltaClock.restart();

            // Handle control events
            while(window.pollEvent(event)){
                ImGui::SFML::ProcessEvent(window, event);

                switch(event.type){
                case sf::Event::Closed:
                    window.close();
                    break;
                default:
                    if(activeScene)
                        activeScene->handleEvent(event);
                    break;
                }
            }

            // Handle updates and stuff
            ImGui::SFML::Update(window, deltaTime);
            if(activeScene)
                activeScene->update(deltaTime);

            // Handle SFML rendering
            window.clear();

            if(activeScene)
                activeScene->render(deltaTime);

            window.display();
        }
    }
}