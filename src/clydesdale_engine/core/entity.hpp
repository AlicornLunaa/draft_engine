#pragma once
#include <entt/entt.hpp>
#include "scene.hpp"

namespace Clydesdale {
    namespace Core {
        class Entity {
        private:
            entt::entity entityID = entt::null;
            Scene* context = nullptr;

        public:
            Entity() = default;
            Entity(Scene* context, entt::entity entityID);
            Entity(const Entity& other) = default;

            template <typename T, typename... Args>
            T& addComponent(Args&&... args){
                T& component = context->getRegistry().emplace<T>(entityID, std::forward<Args>(args)...);
                return component;
            }

            template <typename T>
            T& getComponent(){
                return context->getRegistry().get<T>(entityID);
            }

            template <typename T>
            bool hasComponent(){
                return context->getRegistry().all_of<T>(entityID);
            }

            template <typename T>
            void removeComponent(){
                context->getRegistry().remove<T>(entityID);
            }

            operator entt::entity() const { return entityID; }
        };
    }
}