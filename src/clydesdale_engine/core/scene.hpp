#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

namespace Clydesdale {
    namespace Core {
        class Entity;

        class Scene {
        private:
            sf::RenderWindow* window = nullptr;
            entt::registry registry;

        public:
            Scene(sf::RenderWindow& window);

            entt::registry& getRegistry();
            Entity createEntity();
            void update(sf::Time deltaTime);
            void render(sf::Time deltaTime);
        };
    }
}