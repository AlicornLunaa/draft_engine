#include "draft/ecs/relationship_system.hpp"
#include "draft/ecs/relationship_components.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"

#include <algorithm>
#include <cassert>

namespace Draft {
    void RelationshipSystem::construct_child_func(Registry& reg, entt::entity rawEnt){
        // A child component was attached, get its parent
        ChildComponent& component = reg.get<ChildComponent>(rawEnt);
        Entity& parent = component.parent;

        if(!parent.is_valid())
            // Parent is not a valid entity, exit
            return;

        // Get parents children array
        ParentComponent* childrenCompPtr = nullptr;

        if(!parent.has_component<ParentComponent>()){
            // Parent doesn't have space for children, give it the component
            childrenCompPtr = &parent.add_component<ParentComponent>();
        } else {
            // Parent DOES have space for children, grab its existing array instead
            childrenCompPtr = &parent.get_component<ParentComponent>();
        }

        // Add to array, unless it's already there.
        if(childrenCompPtr){
            Entity self(&m_sceneRef, rawEnt);
            auto& vec = childrenCompPtr->children;

            if(std::find(vec.begin(), vec.end(), self) == vec.end())
                vec.push_back(self);
        }
    }

    void RelationshipSystem::construct_parent_func(Registry& reg, entt::entity rawEnt){
        // A child component was attached, get its parent
        ParentComponent& component = reg.get<ParentComponent>(rawEnt);
        Entity parent(&m_sceneRef, rawEnt);
        auto& vec = component.children;

        // Remove entities which are parented to this
        for(auto& entity : vec){
            if(entity.has_component<ChildComponent>()){
                // Check to see if this owner is correct
                assert(entity.get_component<ChildComponent>().parent == parent && "Entity cannot be parented to two entities");
            } else {
                entity.add_component<ChildComponent>(ChildComponent{parent});
            }
        }
    }

    void RelationshipSystem::deconstruct_child_func(Registry& reg, entt::entity rawEnt){
        // A child component was detached, remove it from the parent
        ChildComponent& component = reg.get<ChildComponent>(rawEnt);
        Entity& parent = component.parent;

        if(!parent.is_valid())
            // Parent is not a valid entity, exit
            return;

        // Get parents children array
        ParentComponent* childrenCompPtr = nullptr;

        if(parent.has_component<ParentComponent>()){
            // Parent DOES have space for children, grab its existing array instead
            childrenCompPtr = &parent.get_component<ParentComponent>();
        }

        // Remove from array
        if(childrenCompPtr){
            auto& vec = childrenCompPtr->children;
            auto iter = std::find(vec.begin(), vec.end(), Entity(&m_sceneRef, rawEnt));

            if(iter != vec.end()){
                vec.erase(iter);
            }

            if(vec.empty()){
                parent.remove_component<ParentComponent>();
            }
        }
    }

    void RelationshipSystem::deconstruct_parent_func(Registry& reg, entt::entity rawEnt){
        // A parent component was detached, destroy all its children
        ParentComponent& component = reg.get<ParentComponent>(rawEnt);
        std::vector<Entity> children = std::move(component.children);
        component.children.clear();

        for(auto& entity : children){
            // Clear the back-reference before destroying
            if(entity.has_component<ChildComponent>())
                entity.get_component<ChildComponent>().parent = Entity();

            entity.destroy();
        }
    }

    // Constructors
    RelationshipSystem::RelationshipSystem(Scene& sceneRef) : m_registryRef(sceneRef.get_registry()), m_sceneRef(sceneRef) {
        // Attach listeners
        m_registryRef.on_construct<ChildComponent>().connect<&RelationshipSystem::construct_child_func>(this);
        m_registryRef.on_construct<ParentComponent>().connect<&RelationshipSystem::construct_parent_func>(this);
        m_registryRef.on_destroy<ChildComponent>().connect<&RelationshipSystem::deconstruct_child_func>(this);
        m_registryRef.on_destroy<ParentComponent>().connect<&RelationshipSystem::deconstruct_parent_func>(this);
    }

    RelationshipSystem::~RelationshipSystem(){
        // Remove listeners
        m_registryRef.on_construct<ChildComponent>().disconnect<&RelationshipSystem::construct_child_func>(this);
        m_registryRef.on_construct<ParentComponent>().disconnect<&RelationshipSystem::construct_parent_func>(this);
        m_registryRef.on_destroy<ChildComponent>().disconnect<&RelationshipSystem::deconstruct_child_func>(this);
        m_registryRef.on_destroy<ParentComponent>().disconnect<&RelationshipSystem::deconstruct_parent_func>(this);
    }
}
