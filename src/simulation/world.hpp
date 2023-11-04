#pragma once
#include <SFML/Graphics.hpp>
#include "cell.hpp"

namespace SpaceGame {
    namespace Simulation {
        class World {
        private:
            Cell** cellArray = nullptr;
            unsigned int width;
            unsigned int height;

            sf::RectangleShape shape;
            float cellSize = 2;

        public:
            World(unsigned int width, unsigned int height);
            ~World();

            void draw(sf::RenderWindow& window);
        };
    }
}