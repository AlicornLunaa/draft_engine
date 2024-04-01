#pragma once

#include "draft/core/registry.hpp"

namespace Draft {
    class Application;
    class Entity;

    class Scene {
    protected:
        Application* app;
        Registry registry;

    public:
        Scene(Application* app);

        Registry& getRegistry();
        Entity createEntity();
        
        // virtual void handleEvent(sf::Event event);
        // virtual void update(sf::Time deltaTime);
        // virtual void render(sf::Time deltaTime);
    };
}