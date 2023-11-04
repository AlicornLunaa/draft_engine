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
            sf::RenderWindow window;
            sf::Event event;
            sf::Clock deltaClock;
            sf::Time deltaTime;
            sf::Time lastTime;

            sf::View uiCamera = sf::View(sf::FloatRect(0, 0, 1280, 720));
            sf::View camera = sf::View(sf::FloatRect(0, 0, 640, 480));

            SpaceGame::Simulation::World world = SpaceGame::Simulation::World(50, 50);

        public:
            Clydesdale::Util::AssetManager assetManager;
            
            Application(const char* title, const unsigned int width, const unsigned int height);
            Application(const Application& rhs) = delete;
            ~Application();

            void run();
        };

    }
}