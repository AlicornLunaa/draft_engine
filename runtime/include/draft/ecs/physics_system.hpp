#pragma once

#include "draft/components/joint_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/ecs/registry.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/ecs/system.hpp"
#include "draft/physics/world.hpp"
#include "draft/util/reflectable.hpp"

#include <algorithm>
#include <cassert>

namespace Draft {
    /**
     * @brief Keeps `RigidBodyComponent`/`ColliderComponent`/joint components in sync with their
     * live box2d-backed native components via reactive `on_construct`/`on_destroy` signals, and
     * steps `World` + syncs forces/joints/bodies every fixed `update(dt)`.
     */
    class PhysicsSystem : public AbstractSystem {
    private:
        // Variables
        Registry& m_registryRef;
        Scene& m_sceneRef;
        World& m_worldRef;

        // Statics
        template<typename T>
        static void construct_joint_wrapper(PhysicsSystem* system, Registry& reg, entt::entity ent){
            system->construct_joint_func<T>(reg, ent);
        }

        template<typename T>
        static void construct_native_joint_wrapper(PhysicsSystem* system, Registry& reg, entt::entity ent){
            system->construct_native_joint_func<T>(reg, ent);
        }

        template<typename T>
        static void deconstruct_joint_wrapper(PhysicsSystem* system, Registry& reg, entt::entity ent){
            system->deconstruct_joint_func<T>(reg, ent);
        }

        template<typename T>
        static void deconstruct_native_joint_wrapper(PhysicsSystem* system, Registry& reg, entt::entity ent){
            system->deconstruct_native_joint_func<T>(reg, ent);
        }

        // Helpers
        template<typename ComponentType>
        void attach_listeners(Registry& registry, PhysicsSystem* system){
            registry.on_construct<ComponentType>().template connect<&PhysicsSystem::construct_joint_wrapper<ComponentType>>(system);
            registry.on_construct<typename ComponentType::NativeType>().template connect<&PhysicsSystem::construct_native_joint_wrapper<ComponentType>>(system);
            registry.on_destroy<ComponentType>().template connect<&PhysicsSystem::deconstruct_joint_wrapper<ComponentType>>(system);
            registry.on_destroy<typename ComponentType::NativeType>().template connect<&PhysicsSystem::deconstruct_native_joint_wrapper<ComponentType>>(system);
        }

        template<typename... ComponentTypes>
        void attach_listeners_for_all(Registry& registry, PhysicsSystem* system){
            (attach_listeners<ComponentTypes>(registry, system), ...);
        }

        template<typename ComponentType>
        void detach_listeners(Registry& registry, PhysicsSystem* system){
            registry.on_construct<ComponentType>().template disconnect<&PhysicsSystem::construct_joint_wrapper<ComponentType>>(system);
            registry.on_construct<typename ComponentType::NativeType>().template disconnect<&PhysicsSystem::construct_native_joint_wrapper<ComponentType>>(system);
            registry.on_destroy<ComponentType>().template disconnect<&PhysicsSystem::deconstruct_joint_wrapper<ComponentType>>(system);
            registry.on_destroy<typename ComponentType::NativeType>().template disconnect<&PhysicsSystem::deconstruct_native_joint_wrapper<ComponentType>>(system);
        }

        template<typename... ComponentTypes>
        void detach_listeners_for_all(Registry& registry, PhysicsSystem* system){
            (detach_listeners<ComponentTypes>(registry, system), ...);
        }

        // Private functions
        void update_transform(Registry& reg, entt::entity rawEnt);

        void construct_body_func(Registry& reg, entt::entity rawEnt);
        void construct_native_body_func(Registry& reg, entt::entity rawEnt);
        void construct_collider_func(Registry& reg, entt::entity rawEnt);

        template<typename T>
        void construct_joint_func(Registry& reg, entt::entity rawEnt){
            // A JointComponent was added, add a new native handle
            T& jointComponent = reg.get<T>(rawEnt);
            Entity self(&m_sceneRef, rawEnt);

            // Make sure the entity doesn't already have a native handle somehow
            if(reg.all_of<typename T::NativeType>(rawEnt))
                reg.remove<typename T::NativeType>(rawEnt);

            // Register the constraint link on both targets, unless it's already there
            if(jointComponent.entityA.is_valid()){
                ConstrainedComponent* constraintComponent = jointComponent.entityA.template has_component<ConstrainedComponent>()
                    ? &jointComponent.entityA.template get_component<ConstrainedComponent>()
                    : &jointComponent.entityA.template add_component<ConstrainedComponent>();

                auto& constraints = constraintComponent->constraints;
                if(std::find(constraints.begin(), constraints.end(), self) == constraints.end())
                    constraints.push_back(self);
            }

            if(jointComponent.entityB.is_valid()){
                ConstrainedComponent* constraintComponent = jointComponent.entityB.template has_component<ConstrainedComponent>()
                    ? &jointComponent.entityB.template get_component<ConstrainedComponent>()
                    : &jointComponent.entityB.template add_component<ConstrainedComponent>();

                auto& constraints = constraintComponent->constraints;
                if(std::find(constraints.begin(), constraints.end(), self) == constraints.end())
                    constraints.push_back(self);
            }

            // Make sure the two target entities have bodies to attach to already; if not, the
            // retry described above finishes the job later.
            if(!jointComponent.entityA.is_valid() || !jointComponent.entityA.template has_component<NativeBodyComponent>())
                return;

            if(!jointComponent.entityB.is_valid() || !jointComponent.entityB.template has_component<NativeBodyComponent>())
                return;

            // Construct actual joint
            reg.emplace<typename T::NativeType>(rawEnt);
        }

        template<typename T>
        void construct_native_joint_func(Registry& reg, entt::entity rawEnt){
            // Make sure the entity has a joint component, otherwise a native joint cant be constructed
            if(!reg.all_of<T>(rawEnt)){
                reg.remove<typename T::NativeType>(rawEnt);
                return;
            }

            // A NativeJointComponent was added, add a new native handle
            typename T::NativeType& nativeHandle = reg.get<typename T::NativeType>(rawEnt);
            T& jointComponent = reg.get<T>(rawEnt);

            // Make sure the two target entities have bodies to attach to
            if(!jointComponent.entityA.is_valid() || !jointComponent.entityA.template has_component<NativeBodyComponent>()){
                reg.remove<typename T::NativeType>(rawEnt);
                return;
            }

            if(!jointComponent.entityB.is_valid() || !jointComponent.entityB.template has_component<NativeBodyComponent>()){
                reg.remove<typename T::NativeType>(rawEnt);
                return;
            }

            // Construct actual joint
            RigidBody* bodyA = jointComponent.entityA.template get_component<NativeBodyComponent>();
            RigidBody* bodyB = jointComponent.entityB.template get_component<NativeBodyComponent>();
            typename T::JointDataType* data = &jointComponent;
            Joint* joint = nullptr;

            // Copy common data to the delta at the native handler
            static_cast<typename T::JointDataType&>(nativeHandle.delta) = *data;

            // Create the joint from the given definition
            using JointDefT = typename JointDefFor<typename T::JointDataType>::Type;
            JointDefT definition(bodyA, bodyB, data->collideConnected);
            static_cast<typename T::JointDataType&>(definition) = *data;
            joint = m_worldRef.create_joint(definition);
            assert(joint && "Something went wrong with a joint");

            // Add a handle to the entity so it can be referenced later
            nativeHandle.jointPtr = joint;
            jointComponent.m_nativeHandlePtr = joint;
        }

        void deconstruct_body_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_native_body_func(Registry& reg, entt::entity rawEnt);
        void deconstruct_collider_func(Registry& reg, entt::entity rawEnt);

        template<typename T>
        void deconstruct_joint_func(Registry& reg, entt::entity rawEnt){
            // Make sure it has a native handle and remove it
            if(reg.all_of<typename T::NativeType>(rawEnt))
                reg.remove<typename T::NativeType>(rawEnt);

            // Remove the constraints for other entities
            T& joint = reg.get<T>(rawEnt);
            Entity currentEntity = Entity(&m_sceneRef, rawEnt);

            // Entity A
            if(joint.entityA.is_valid() && joint.entityA.template has_component<ConstrainedComponent>()){
                auto& constraints = joint.entityA.template get_component<ConstrainedComponent>().constraints;

                // Find and remove entity reference
                auto iter = std::find(constraints.begin(), constraints.end(), currentEntity);
                if(iter != constraints.end()){
                    constraints.erase(iter);
                }

                // If the constraints are empty, remove the component too
                if(constraints.empty()){
                    joint.entityA.template remove_component<ConstrainedComponent>();
                }
            }

            // Entity B
            if(joint.entityB.is_valid() && joint.entityB.template has_component<ConstrainedComponent>()){
                auto& constraints = joint.entityB.template get_component<ConstrainedComponent>().constraints;

                // Find and remove entity reference
                auto iter = std::find(constraints.begin(), constraints.end(), currentEntity);
                if(iter != constraints.end()){
                    constraints.erase(iter);
                }

                // If the constraints are empty, remove the component too
                if(constraints.empty()){
                    joint.entityB.template remove_component<ConstrainedComponent>();
                }
            }
        }

        template<typename T>
        void deconstruct_native_joint_func(Registry& reg, entt::entity rawEnt){
            // A joint component was removed, delete the world joint
            typename T::NativeType& nativeComponent = reg.get<typename T::NativeType>(rawEnt);

            // Make sure the joint is valid since its a pointer
            if(nativeComponent.jointPtr)
                // Destroy it
                nativeComponent->destroy();

            // Remove dangling pointer from the data component
            if(reg.all_of<T>(rawEnt)){
                auto& jointComponent = reg.get<T>(rawEnt);
                jointComponent.m_nativeHandlePtr = nullptr;
            }
        }

        void handle_bodies();
        void handle_forces();

        template<typename T>
        void sync_joint_type();

        template<typename... Ts>
        void sync_joint_types(){
            (sync_joint_type<Ts>(), ...);
        }

        void handle_joints();

    public:
        // Constructors
        PhysicsSystem(Scene& sceneRef, World& worldRef);
        PhysicsSystem(PhysicsSystem&& other) = delete;
        PhysicsSystem(const PhysicsSystem& other) = delete;
        ~PhysicsSystem() override;

        // Operators
        PhysicsSystem& operator=(PhysicsSystem&& other) = delete;
        PhysicsSystem& operator=(const PhysicsSystem& other) = delete;

        // Functions
        void update(Time dt) override;

        // No fields of its own to save, m_registryRef/m_sceneRef/m_worldRef are construction
        // dependencies (see SystemFactory), not tunable state. Empty so PhysicsSystem can still
        // be registered via SystemCatalog::register_system<T>() (requires Reflectable<T>).
        DRAFT_REFLECTABLE(PhysicsSystem)
    };
};
