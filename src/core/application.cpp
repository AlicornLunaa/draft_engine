#include "draft/core/application.hpp"
#include "draft/interface/panel.hpp"
#include "draft/interface/button.hpp"
#include "draft/interface/element.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    Application::Application(const char* title, const unsigned int width, const unsigned int height)
            : window(sf::RenderWindow(sf::VideoMode(width, height), title)), width(width), height(height) {
        // Feedback
        Logger::println(Level::INFO, "Draft Engine", "Initializing...");
        
        // Make sure game can run, handle arguments, etc
        if(!sf::Shader::isAvailable()){
            Logger::println(Level::CRITICAL, "Draft Engine", "Shaders unavailable, OpenGL is too old? Cannot continue.");
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
    }

    Application::~Application(){
        // Cleanup
        Logger::println(Level::INFO, "Draft Engine", "Exitting...");
        ImGui::SFML::Shutdown();
    }

    void Application::handleEvent(){
        if(activeScene){
            activeScene->handleEvent(event);
        }
    }

    void Application::run(){
        Element testElement = Element(150, 50, 300, 150);
        Panel newElement = Panel(testElement, sf::Color::Red, 5, 5, 290, 20);
        Button btn = Button(testElement, sf::Color::Cyan, assetManager.getFont("./assets/fonts/default.ttf"), "Test Button", 5, 5, 290, 20);

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
            if(activeScene)
                activeScene->update(deltaTime);

            // Handle ImGUI rendering
            ImGui::ShowDemoWindow();
            ImGui::Begin("Stats");
            ImGui::Text("FPS: %02d", (int)(1.f / deltaTime.asSeconds()));
            ImGui::Text("Frame time: %f", deltaTime.asSeconds());
            if(ImGui::Button("Reload Assets"))
                assetManager.reload();
            if(ImGui::Button("Quit"))
                window.close();
            ImGui::End();

            // Handle SFML rendering
            window.clear();

            if(activeScene)
                activeScene->render(deltaTime);

            window.setView(imGuiCamera);
            ImGui::SFML::Render(window);

            window.display();
        }
    }
}