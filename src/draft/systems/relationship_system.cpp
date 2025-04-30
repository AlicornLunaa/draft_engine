#include "draft/components/relationship_components.hpp"
#include "draft/systems/relationship_system.hpp"
#include "draft/core/entity.hpp"
#include "draft/core/scene.hpp"
#include <algorithm>
#include <cassert>

namespace Draft {
    // Private functions
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

        // Add to array
        if(childrenCompPtr){
            childrenCompPtr->children.push_back(Entity(&m_sceneRef, rawEnt));
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
                entity.add_component<ChildComponent>();
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
        auto& vec = component.children;

        // Remove entities which are parented to this
        for(auto& entity : vec){
            entity.destroy();
        }
        
        vec.clear();
    }

    // Constructors
    RelationshipSystem::RelationshipSystem(Scene& sceneRef) : m_sceneRef(sceneRef), m_registryRef(sceneRef.get_registry()) {
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
};