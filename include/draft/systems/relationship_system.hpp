#pragma once

#include "draft/core/registry.hpp"

namespace Draft {
    /// Forward decl
    class Scene;

    /// Handles child <--> parent relationship components, should ALWAYS be created with default scenes.
    class RelationshipSystem {
    private:
        // Variables
        Registry& m_registryRef;
        Scene& m_sceneRef;

        // Private functions
        void construct_child_func(Registry& reg, entt::entity rawEnt); // If a child is created, let its parent know
        void construct_parent_func(Registry& reg, entt::entity rawEnt); // If a parent is created, let its children know
        void deconstruct_child_func(Registry& reg, entt::entity rawEnt); // If a child is destroyed, let its parent know
        void deconstruct_parent_func(Registry& reg, entt::entity rawEnt); // If a parent is destroyed, delete the children

    public:
        // Constructors
        RelationshipSystem(Scene& sceneRef);
        RelationshipSystem(const RelationshipSystem& other) = delete;
        RelationshipSystem(RelationshipSystem&& other) = delete;
        ~RelationshipSystem();

        // Operators
        RelationshipSystem& operator=(const RelationshipSystem& other) = delete;
        RelationshipSystem& operator=(RelationshipSystem&& other) = delete;
    };
};