#pragma once

#include "entt/entt.hpp"
#include "scene.hpp"
#include <functional>

namespace Draft {
    /**
     * @brief EnTT entity which holds a pointer to the scene its a part of.
     */
    class Entity {
    private:
        entt::entity m_entityID = entt::null;
        Scene* m_context = nullptr;

    public:
        Entity() = default;
        Entity(Scene* context, entt::entity entityID);
        Entity(const Entity& other) = default;

        bool is_valid() const;
        Scene* get_scene();

        /**
         * @brief Removes the entity from the scene, nullifying it
         * @return bool
         */
        bool destroy();

        /**
         * @brief Adds a component to the entity
         * @tparam T
         * @tparam Args
         * @param args Constructor arguments forwarded to the component
         * @return T& 
         */
        template<typename T, typename... Args>
        T& add_component(Args&&... args){
            T& component = m_context->get_registry().emplace<T>(m_entityID, std::forward<Args>(args)...);
            return component;
        }

        /**
         * @brief Get the component object
         * @tparam T 
         * @return T& 
         */
        template<typename T>
        T& get_component(){
            return m_context->get_registry().get<T>(m_entityID);
        }

        /**
         * @brief Get the component object as a pointer, nullptr if none
         * @tparam T
         * @return T*
         */
        template<typename T>
        T* try_get_component(){
            if(!m_context) return nullptr;
            if(m_entityID == entt::null) return nullptr;
            return m_context->get_registry().try_get<T>(m_entityID);
        }

        /**
         * @brief Modifies the component and triggers on_update
         * @tparam T 
         * @param patchFunc 
         */
        template<typename T>
        void modify_component(std::function<void(T& component)> patchFunc){
            m_context->get_registry().patch<T>(m_entityID, patchFunc);
        }

        /**
         * @brief Checks if the component is a part of the entity
         * @tparam T 
         * @return true 
         * @return false 
         */
        template<typename ...T>
        bool has_component(){
            return m_context->get_registry().all_of<T...>(m_entityID);
        }

        /**
         * @brief Removes a component from the entity
         * @tparam T 
         */
        template<typename T>
        void remove_component(){
            m_context->get_registry().remove<T>(m_entityID);
        }

        operator bool() const { return is_valid(); }
        operator entt::entity() const { return m_entityID; }
        bool operator==(const Entity& other) const { return (m_entityID == other.m_entityID); }
    };

    const Entity NULL_ENTITY = Entity(nullptr, entt::null);
}