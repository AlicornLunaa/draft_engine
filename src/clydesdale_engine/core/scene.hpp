#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <clydesdale_engine/util/asset_manager.hpp>

namespace Clydesdale {
    namespace Core {
        class Entity;

        class Scene {
        protected:
            sf::RenderWindow* window = nullptr;
            entt::registry registry;

        public:
            Scene(Util::AssetManager& assetManager, sf::RenderWindow& window);

            entt::registry& getRegistry();
            Entity createEntity();
            
            virtual void handleEvent(sf::Event event);
            virtual void update(sf::Time deltaTime);
            virtual void render(sf::Time deltaTime);
        };
    }
}