#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

namespace Draft {
    class Application;
    class Entity;

    class Scene {
    protected:
        Application* app;
        entt::registry registry;
        sf::View uiCamera;

    public:
        Scene(Application* app);

        entt::registry& getRegistry();
        Entity createEntity();
        
        virtual void handleEvent(sf::Event event);
        virtual void update(sf::Time deltaTime);
        virtual void render(sf::Time deltaTime);
    };
}