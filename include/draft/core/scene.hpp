#pragma once

#include "draft/core/registry.hpp"
#include "draft/util/time.hpp"

namespace Draft {
    class Application;
    class Entity;

    class Scene {
    protected:
        Application* app;
        Registry registry;

    public:
        Scene(Application* app);

        Registry& get_registry();
        Entity create_entity();
        
        // virtual void handleEvent(sf::Event event);
        virtual void update(Time deltaTime);
        virtual void render(Time deltaTime);
    };
}