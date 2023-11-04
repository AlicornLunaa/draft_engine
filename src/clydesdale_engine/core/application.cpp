#include "application.hpp"

using namespace Clydesdale::Core;
using namespace Clydesdale::Util;

Application::Application(const char* title, const unsigned int width, const unsigned int height)
        : window(sf::RenderWindow(sf::VideoMode(width, height), title)), width(width), height(height) {
    // Feedback
    Logger::println(Logger::Level::INFO, "Clydesdale", "Initializing...");
    
    // Make sure game can run, handle arguments, etc
    if(!sf::Shader::isAvailable()){
        Util::Logger::println(Util::Logger::Level::CRITICAL, "Clydesdale", "Shaders unavailable, OpenGL is too old? Cannot continue.");
        exit(1);
    }

    // ImGUI
    if(!ImGui::SFML::Init(window)){
        Logger::println(Logger::Level::CRITICAL, "ImGUI", "Failed to initialize, exitting.");
        exit(1);
    }
    auto& imGuiIO = ImGui::GetIO();
    imGuiIO.IniFilename = nullptr;
    imGuiIO.LogFilename = nullptr;
}

Application::~Application(){
    // Cleanup
    Logger::println(Logger::Level::INFO, "Clydesdale", "Exitting...");
    ImGui::SFML::Shutdown();
}

void Application::run(){
    // Start game loop
    while(window.isOpen()){
        // Handle control events
        while(window.pollEvent(event)){
            ImGui::SFML::ProcessEvent(window, event);

            switch(event.type){
            case sf::Event::Closed:
                window.close();
                break;
            default:
                handleEvent();
                break;
            }
        }

        deltaTime = deltaClock.restart();
        ImGui::SFML::Update(window, deltaTime);

        // Handle ImGUI rendering
        ImGui::Begin("Stats");
        ImGui::Text("FPS: %02d", (int)(1.f / deltaTime.asSeconds()));
        ImGui::End();

        // Handle SFML rendering
        window.clear();
        draw();
        window.setView(imGuiCamera);
        ImGui::SFML::Render(window);
        window.display();
    }
}