#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include <clydesdale_engine/util/logger.hpp>
#include <clydesdale_engine/util/asset_manager.hpp>
#include <space_game/simulation/world.hpp>

namespace Clydesdale {
    namespace Core {
        
        class Application {
        private:
            unsigned int width, height;
            sf::View imGuiCamera = sf::View(sf::FloatRect(0, 0, width, height));

        public:
            Clydesdale::Util::AssetManager assetManager;
            sf::RenderWindow window;
            sf::Clock deltaClock;
            sf::Time deltaTime;
            sf::Event event;
            
            Application(const char* title, const unsigned int width, const unsigned int height);
            Application(const Application& rhs) = delete;
            ~Application();

            virtual void handleEvent() = 0;
            virtual void init() = 0;
            virtual void draw() = 0;
            void run();
        };

    }
}