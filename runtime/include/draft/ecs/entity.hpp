#pragma once

#include "draft/ecs/scene.hpp"
#include "entt/entt.hpp"

#include <cassert>
#include <functional>

namespace Draft {
    /**
     * @brief A lightweight handle to an entity within a specific Scene: an EnTT entity ID plus
     * the Scene it belongs to.
     */
    class Entity {
    private:
        entt::entity m_entityID = entt::null;
        Scene* m_context = nullptr;

    public:
        Entity() = default;
        Entity(Scene* context, entt::entity entityID);
        Entity(const Entity& other) = default;

        /**
         * @brief True if this handle refers to a real scene and entity ID. Does not check
         * whether the entity itself still exists in that scene - see Scene::get_registry()
         * and entt::registry::valid() for that.
         */
        bool is_valid() const;

        Scene* get_scene();

        /**
         * @brief Destroys the entity in its scene and nulls out this handle.
         * @return False if this handle was already invalid.
         */
        bool destroy();

        /**
         * @brief Adds a component to the entity.
         */
        template<typename T, typename... Args>
        T& add_component(Args&&... args){
            assert(is_valid() && "Entity::add_component() called on an invalid entity");
            return m_context->get_registry().emplace<T>(m_entityID, std::forward<Args>(args)...);
        }

        /**
         * @brief Gets a component already known to exist on the entity.
         */
        template<typename T>
        T& get_component(){
            assert(is_valid() && "Entity::get_component() called on an invalid entity");
            return m_context->get_registry().get<T>(m_entityID);
        }

        /**
         * @brief Gets a component if it exists, or nullptr otherwise. Unlike the other
         * component accessors, this is safe to call on an invalid (or possibly-stale) handle in
         * every build configuration - see the class-level comment.
         */
        template<typename T>
        T* try_get_component(){
            if(!m_context || m_entityID == entt::null)
                return nullptr;

            return m_context->get_registry().try_get<T>(m_entityID);
        }

        /**
         * @brief Modifies the component in place and triggers the registry's on_update signal.
         */
        template<typename T>
        void modify_component(std::function<void(T& component)> patchFunc){
            assert(is_valid() && "Entity::modify_component() called on an invalid entity");
            m_context->get_registry().patch<T>(m_entityID, patchFunc);
        }

        /**
         * @brief Checks whether the entity has every one of the given component types.
         */
        template<typename ...T>
        bool has_component(){
            assert(is_valid() && "Entity::has_component() called on an invalid entity");
            return m_context->get_registry().all_of<T...>(m_entityID);
        }

        /**
         * @brief Removes a component from the entity.
         */
        template<typename T>
        void remove_component(){
            assert(is_valid() && "Entity::remove_component() called on an invalid entity");
            m_context->get_registry().remove<T>(m_entityID);
        }

        operator bool() const { return is_valid(); }
        operator entt::entity() const { return m_entityID; }

        /**
         * @brief Two handles are equal only if they refer to the same entity ID *and* the same
         * scene - entt::entity values are small, scene-local, recycled integers, so two
         * entities from different scenes can otherwise share the same numeric ID.
         */
        bool operator==(const Entity& other) const { return m_context == other.m_context && m_entityID == other.m_entityID; }
    };

    /**
     * @brief A named, explicit null entity - equivalent to a default-constructed Entity{}, kept
     * around as a readable sentinel for comparisons (`entity == NULL_ENTITY`) and defaults.
     */
    const Entity NULL_ENTITY = Entity(nullptr, entt::null);
}
