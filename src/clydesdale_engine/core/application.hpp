#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include <clydesdale_engine/core/scene.hpp>
#include <clydesdale_engine/util/logger.hpp>
#include <clydesdale_engine/util/asset_manager.hpp>

namespace Clydesdale {
    namespace Core {
        class Application {
        private:
            unsigned int width, height;
            sf::View imGuiCamera = sf::View(sf::FloatRect(0, 0, width, height));
            Scene* activeScene = nullptr;

        public:
            Util::AssetManager assetManager;
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
}