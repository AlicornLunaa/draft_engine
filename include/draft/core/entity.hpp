#pragma once

#include "entt/entt.hpp"
#include "scene.hpp"

namespace Draft {
    /**
     * @brief EnTT entity which holds a pointer to the scene its a part of.
     */
    class Entity {
    private:
        entt::entity entityID = entt::null;
        Scene* context = nullptr;

    public:
        Entity() = default;
        Entity(Scene* context, entt::entity entityID);
        Entity(const Entity& other) = default;

        /**
         * @brief Adds a component to the entity
         * @tparam T
         * @tparam Args
         * @param args Constructor arguments forwarded to the component
         * @return T& 
         */
        template <typename T, typename... Args>
        T& add_component(Args&&... args){
            T& component = context->get_registry().emplace<T>(entityID, std::forward<Args>(args)...);
            return component;
        }

        /**
         * @brief Get the component object
         * @tparam T 
         * @return T& 
         */
        template <typename T>
        T& get_component(){
            return context->get_registry().get<T>(entityID);
        }

        /**
         * @brief Checks if the component is a part of the entity
         * @tparam T 
         * @return true 
         * @return false 
         */
        template <typename T>
        bool has_component(){
            return context->get_registry().all_of<T>(entityID);
        }

        /**
         * @brief Removes a component from the entity
         * @tparam T 
         */
        template <typename T>
        void remove_component(){
            context->get_registry().remove<T>(entityID);
        }

        operator entt::entity() const { return entityID; }
        bool operator==(const Entity& other) const { return (entityID == other.entityID); }
    };
}