#include "application.hpp"

using namespace Clydesdale::Core;
using namespace Clydesdale::Util;

Application::Application(const char* title, const unsigned int width, const unsigned int height)
        : window(sf::RenderWindow(sf::VideoMode(width, height), title)) {
    // Feedback
    Logger::println(Logger::Level::INFO, "Space Game", "Initializing...");

    // Assets
    assetManager.queueTexture("./assets/textures/test_image_1.png");
    assetManager.queueTexture("./assets/textures/test_image_2.jpg");
    assetManager.queueShader("./assets/shaders/default");
    assetManager.queueShader("./assets/shaders/invert");
    assetManager.queueAudio("./assets/audio/boo_womp.mp3");
    assetManager.load();

    // Window settings
    window.setFramerateLimit(60);

    // ImGUI
    if(!ImGui::SFML::Init(window)){
        Logger::println(Logger::Level::CRITICAL, "ImGUI", "Failed to initialize, exitting.");
        exit(1);
    }

    // Final feedback
    Logger::println(Logger::Level::INFO, "Space Game", "Finished");
}

Application::~Application(){
    // Cleanup
    Logger::println(Logger::Level::INFO, "Space Game", "Exitting...");
    ImGui::SFML::Shutdown();
}

void Application::run(){
    // Basic allocations
    const sf::SoundBuffer& soundBuffer = assetManager.getAudio("./assets/audio/boo_womp.mp3");
    sf::Texture& texture1 = assetManager.getTexture("./assets/textures/test_image_1.png");
    texture1.setSmooth(false);

    sf::Sprite sprite(texture1);
    sf::Sound sound(soundBuffer);

    // Start game loop
    while(window.isOpen()){
        // Handle control events
        while(window.pollEvent(event)){
            ImGui::SFML::ProcessEvent(window, event);

            switch(event.type){
            case sf::Event::Closed:
                window.close();
                break;
            }
        }

        lastTime = deltaTime;
        deltaTime = deltaClock.restart();
        ImGui::SFML::Update(window, deltaTime);

        // Handle ImGUI rendering
        // ImGui::ShowDemoWindow();

        ImGui::Begin("Hello, world!");
        if(ImGui::Button("Look at this pretty button"))
            assetManager.reload();
        if(ImGui::Button("Turn off my boo-womp inhibitors!"))
            sound.play();
        ImGui::End();
        
        ImGui::Begin("Stats");
        ImGui::Text("FPS: %02d", (int)(1.f / deltaTime.asSeconds()));
        ImGui::End();

        // Handle SFML rendering
        window.clear();
        window.draw(sprite);
        world.draw(window);
        ImGui::SFML::Render(window);
        window.display();
    }
}