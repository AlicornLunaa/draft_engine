#pragma once

#include "draft/ecs/registry.hpp"
#include "draft/ecs/relationship_system.hpp"
#include "draft/ecs/system.hpp"
#include "draft/input/event.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/util/time.hpp"
#include <memory>

namespace Draft {
    class Entity;

    /**
     * @brief A pure ECS container. An entity registry, entity creation, parent/child
     * relationships (RelationshipSystem), and whatever per-tick systems are registered against
     * it (SystemRegistry)
     */
    class Scene {
    public:
        Scene();
        Scene(const Scene& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        ~Scene() = default;

        Registry& get_registry();
        const Registry& get_registry() const;

        SystemRegistry& get_systems();
        const SystemRegistry& get_systems() const;

        /**
         * @brief Creates a new, component-less entity in this scene.
         */
        Entity create_entity();

        /**
         * @brief Resolves the highest-priority active CameraComponent in this scene, syncing its
         * Camera's position/rotation from that entity's TransformComponent (if any) first.
         * Returns nullptr if no active CameraComponent exists. Never save this pointer, it may change or be freed.
         */
        Camera* get_active_camera();

        /**
         * @brief Set the active camera object
         * @param camera 
         */
        void set_active_camera_override(std::unique_ptr<Camera>&& camera);

        /**
         * @brief Advances every registered system by a fixed-size @p dt, in registration order.
         * Equivalent to get_systems().update_all(dt). Meant to be called zero or more times per
         * frame from an accumulator loop (deterministic physics among the reasons why)
         * see render() for the once-per-frame, variable-dt counterpart.
         */
        void update(Time dt);

        /**
         * @brief Runs every registered system's per-frame work with the actual, variable frame
         * @p dt for the given @p layer, in registration order. Equivalent to get_systems().render_all(dt, layer).
         */
        void render(Time dt, RenderLayer layer);

        /**
         * @brief Notifies every registered system that this Scene has become the active one.
         * Equivalent to get_systems().attach_all().
         */
        void attach();

        /**
         * @brief Notifies every registered system that this Scene has stopped being the active
         * one. Equivalent to get_systems().detach_all().
         */
        void detach();

        /**
         * @brief Dispatches @p event to every registered system in registration order, stopping
         * at the first one that consumes it. Equivalent to get_systems().dispatch_event(event).
         * @return True if some system consumed the event.
         */
        bool dispatch_event(const Event& event);

    private:
        // Declaration order matters, m_registry must exist before m_relationshipSystem is
        // constructed, since its constructor calls get_registry() on this Scene.
        Registry m_registry;
        RelationshipSystem m_relationshipSystem;
        SystemRegistry m_systems;

        std::unique_ptr<Camera> m_cameraOverride; // Used to override scene camera without an entity
    };
}
