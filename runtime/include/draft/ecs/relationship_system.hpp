#pragma once

#include "draft/ecs/registry.hpp"

namespace Draft {
    class Scene;

    /**
     * @brief Keeps ChildComponent/ParentComponent in sync as entities are parented/unparented,
     * via EnTT's on_construct/on_destroy signals. Owned directly by Scene (not through
     * SystemRegistry), This reacts to component construction/destruction, it has no per-tick
     * work of its own.
     */
    class RelationshipSystem {
    private:
        Registry& m_registryRef;
        Scene& m_sceneRef;

        void construct_child_func(Registry& reg, entt::entity rawEnt);
        void construct_parent_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_child_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_parent_func(Registry& reg, entt::entity rawEnt);

    public:
        RelationshipSystem(Scene& sceneRef);
        RelationshipSystem(const RelationshipSystem& other) = delete;
        RelationshipSystem(RelationshipSystem&& other) = delete;
        ~RelationshipSystem();

        RelationshipSystem& operator=(const RelationshipSystem& other) = delete;
        RelationshipSystem& operator=(RelationshipSystem&& other) = delete;
    };
}
