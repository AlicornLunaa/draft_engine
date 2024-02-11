#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include "clydesdale/util/asset_manager.hpp"

namespace Clydesdale {
    class Entity;

    class Scene {
    protected:
        sf::RenderWindow* window = nullptr;
        entt::registry registry;

    public:
        Scene(AssetManager& assetManager, sf::RenderWindow& window);

        entt::registry& getRegistry();
        Entity createEntity();
        
        virtual void handleEvent(sf::Event event);
        virtual void update(sf::Time deltaTime);
        virtual void render(sf::Time deltaTime);
    };
}