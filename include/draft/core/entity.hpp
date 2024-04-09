#pragma once

#include "entt/entt.hpp"
#include "scene.hpp"

namespace Draft {
    class Entity {
    private:
        entt::entity entityID = entt::null;
        Scene* context = nullptr;

    public:
        Entity() = default;
        Entity(Scene* context, entt::entity entityID);
        Entity(const Entity& other) = default;

        template <typename T, typename... Args>
        T& add_component(Args&&... args){
            T& component = context->get_registry().emplace<T>(entityID, std::forward<Args>(args)...);
            return component;
        }

        template <typename T>
        T& get_component(){
            return context->get_registry().get<T>(entityID);
        }

        template <typename T>
        bool has_component(){
            return context->get_registry().all_of<T>(entityID);
        }

        template <typename T>
        void remove_component(){
            context->get_registry().remove<T>(entityID);
        }

        operator entt::entity() const { return entityID; }
    };
}