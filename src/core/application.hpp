#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "../util/logger.hpp"
#include "../util/asset_manager.hpp"
#include "../simulation/world.hpp"

namespace Krypton {
    namespace Core {
        class Application {
        private:
            sf::RenderWindow window;
            sf::Event event;
            sf::Clock deltaClock;
            sf::Time deltaTime;
            sf::Time lastTime;

            Simulation::World world = Simulation::World(50, 50);

        public:
            Krypton::Util::AssetManager assetManager;
            
            Application(const char* title, const unsigned int width, const unsigned int height);
            Application(const Application& rhs) = delete;
            ~Application();

            void run();
        };
    }
}