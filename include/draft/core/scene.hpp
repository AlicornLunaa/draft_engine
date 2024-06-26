#pragma once

#include "draft/core/registry.hpp"
#include "draft/input/event.hpp"
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
        Scene(const Scene& other) = delete;

        Registry& get_registry();
        Entity create_entity();
        
        /**
         * @brief Called when an event is polled from the window
         * @param event 
         */
        virtual void handleEvent(Event event);

        /**
         * @brief Called in a fixed time step which is set in the application
         * @param deltaTime 
         */
        virtual void update(Time deltaTime);

        /**
         * @brief Called every frame
         * @param deltaTime 
         */
        virtual void render(Time deltaTime);
    };
}