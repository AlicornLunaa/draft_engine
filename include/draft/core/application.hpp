#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "draft/core/scene.hpp"
#include "draft/util/asset_manager.hpp"

namespace Draft {
    class Application {
    private:
        unsigned int width, height;
        Scene* activeScene = nullptr;
        sf::View imGuiCamera = sf::View(sf::FloatRect(0, 0, width, height));

    public:
        AssetManager assetManager;
        sf::RenderWindow window;
        sf::Clock deltaClock;
        sf::Time deltaTime;
        sf::Event event;
        
        Application(const char* title, const unsigned int width, const unsigned int height);
        Application(const Application& rhs) = delete;
        ~Application();

        void handleEvent();
        void run();

        void setScene(Scene* scene){ activeScene = scene; }
        Scene* getScene(){ return activeScene; }

        unsigned int getWidth(){ return width; }
        unsigned int getHeight(){ return height; }
    };
}