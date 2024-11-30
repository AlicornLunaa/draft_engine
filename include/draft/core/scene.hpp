#pragma once

#include "draft/core/registry.hpp"
#include "draft/input/event.hpp"
#include "draft/util/time.hpp"

namespace Draft {
    class Application;
    class Entity;
    class PhysicsSystem;

    class Scene {
    protected:
        Application* app;
        Registry registry;

    public:
        Scene(Application* app);
        Scene(const Scene& other) = delete;
        virtual ~Scene() = default;

        Application* get_app();
        Registry& get_registry();
        Entity create_entity();
        
        /**
         * @brief Called when an event is polled from the window
         * @param event 
         */
        virtual void handle_event(Event event);

        /**
         * @brief Called in a fixed time step which is set in the application
         * @param timeStep 
         */
        virtual void update(Time timeStep);

        /**
         * @brief Called every frame
         * @param deltaTime 
         */
        virtual void render(Time deltaTime);

        /**
         * @brief Called when the scene is set to the current scene
         */
        virtual void on_attach();

        /**
         * @brief Called when the scene is removed from the current scene
         */
        virtual void on_detach();
    };
}