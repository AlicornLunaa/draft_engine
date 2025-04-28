#pragma once

#include "draft/components/joint_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/core/application.hpp"
#include "draft/core/registry.hpp"
#include "draft/core/scene.hpp"
#include "draft/phys/world.hpp"

namespace Draft {
    class PhysicsSystem {
    private:
        // Variables
        Application* m_appPtr = nullptr;
        Registry& m_registryRef;
        Scene& m_sceneRef;

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
        void construct_body_func(Registry& reg, entt::entity rawEnt);
        void construct_native_body_func(Registry& reg, entt::entity rawEnt);
        void construct_collider_func(Registry& reg, entt::entity rawEnt);
        
        template<typename T>
        void construct_joint_func(Registry& reg, entt::entity rawEnt){
            // A JointComponent was added, add a new native handle
            T& jointComponent = reg.get<T>(rawEnt);

            // Make sure the entity doesn't already have a native handle somehow
            if(reg.all_of<typename T::NativeType>(rawEnt))
                reg.remove<typename T::NativeType>(rawEnt);

            // Make sure the two target entities have bodies to attach to
            if(!jointComponent.entityA.is_valid() || !jointComponent.entityA.template has_component<NativeBodyComponent>())
                return;

            if(!jointComponent.entityB.is_valid() || !jointComponent.entityB.template has_component<NativeBodyComponent>())
                return;
            
            // Construct actual joint
            reg.emplace<typename T::NativeType>(rawEnt);

            // Add constraint references to the targets
            // Entity A
            ConstrainedComponent* constraintComponent = nullptr;
            
            if(!jointComponent.entityA.template has_component<ConstrainedComponent>()){
                constraintComponent = &jointComponent.entityA.template add_component<ConstrainedComponent>();
            } else {
                constraintComponent = &jointComponent.entityA.template get_component<ConstrainedComponent>();
            }

            constraintComponent->constraints.push_back(Entity(&m_sceneRef, rawEnt));

            // Entity B
            if(!jointComponent.entityB.template has_component<ConstrainedComponent>()){
                constraintComponent = &jointComponent.entityB.template add_component<ConstrainedComponent>();
            } else {
                constraintComponent = &jointComponent.entityB.template get_component<ConstrainedComponent>();
            }

            constraintComponent->constraints.push_back(Entity(&m_sceneRef, rawEnt));
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

            // TODO: Flatten this into static polymorphism using overloading, now that its templated
            if(auto* ptr = dynamic_cast<DistanceJointData*>(data)){
                DistanceJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.anchorA = ptr->anchorA;
                definition.anchorB = ptr->anchorB;
                definition.length = ptr->length;
                definition.minLength = ptr->minLength;
                definition.maxLength = ptr->maxLength;
                definition.stiffness = ptr->stiffness;
                definition.damping = ptr->damping;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<RevoluteJointData*>(data)){
                RevoluteJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.localAnchorA = ptr->localAnchorA;
                definition.localAnchorB = ptr->localAnchorB;
                definition.referenceAngle = ptr->referenceAngle;
                definition.lowerAngle = ptr->lowerAngle;
                definition.upperAngle = ptr->upperAngle;
                definition.maxMotorTorque = ptr->maxMotorTorque;
                definition.motorSpeed = ptr->motorSpeed;
                definition.enableLimit = ptr->enableLimit;
                definition.enableMotor = ptr->enableMotor;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<PrismaticJointData*>(data)){
                PrismaticJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.anchorA = ptr->anchorA;
                definition.anchorB = ptr->anchorB;
                definition.localAxisA = ptr->localAxisA;
                definition.referenceAngle = ptr->referenceAngle;
                definition.lowerTranslation = ptr->lowerTranslation;
                definition.upperTranslation = ptr->upperTranslation;
                definition.maxMotorForce = ptr->maxMotorForce;
                definition.motorSpeed = ptr->motorSpeed;
                definition.enableLimit = ptr->enableLimit;
                definition.enableMotor = ptr->enableMotor;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<PulleyJointData*>(data)){
                PulleyJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.groundAnchorA = ptr->groundAnchorA;
                definition.groundAnchorB = ptr->groundAnchorB;
                definition.localAnchorA = ptr->localAnchorA;
                definition.localAnchorB = ptr->localAnchorB;
                definition.lengthA = ptr->lengthA;
                definition.lengthB = ptr->lengthB;
                definition.ratio = ptr->ratio;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<GearJointData*>(data)){
                GearJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.joint1 = ptr->joint1;
                definition.joint2 = ptr->joint2;
                definition.ratio = ptr->ratio;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<MouseJointData*>(data)){
                MouseJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.target = ptr->target;
                definition.maxForce = ptr->maxForce;
                definition.stiffness = ptr->stiffness;
                definition.damping = ptr->damping;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<WheelJointData*>(data)){
                WheelJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.anchorA = ptr->anchorA;
                definition.anchorB = ptr->anchorB;
                definition.localAxis = ptr->localAxis;
                definition.lowerTranslation = ptr->lowerTranslation;
                definition.upperTranslation = ptr->upperTranslation;
                definition.maxMotorTorque = ptr->maxMotorTorque;
                definition.motorSpeed = ptr->motorSpeed;
                definition.stiffness = ptr->stiffness;
                definition.damping = ptr->damping;
                definition.enableLimit = ptr->enableLimit;
                definition.enableMotor = ptr->enableMotor;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<WeldJointData*>(data)){
                WeldJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.anchorA = ptr->anchorA;
                definition.anchorB = ptr->anchorB;
                definition.referenceAngle = ptr->referenceAngle;
                definition.stiffness = ptr->stiffness;
                definition.damping = ptr->damping;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<FrictionJointData*>(data)){
                FrictionJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.anchorA = ptr->anchorA;
                definition.anchorB = ptr->anchorB;
                definition.maxForce = ptr->maxForce;
                definition.maxTorque = ptr->maxTorque;
                joint = world.create_joint(definition);
            } else if(auto* ptr = dynamic_cast<MotorJointData*>(data)){
                MotorJointDef definition(bodyA, bodyB, data->collideConnected);
                definition.linearOffset = ptr->linearOffset;
                definition.angularOffset = ptr->angularOffset;
                definition.maxForce = ptr->maxForce;
                definition.maxTorque = ptr->maxTorque;
                definition.correctionFactor = ptr->correctionFactor;
                joint = world.create_joint(definition);
            }

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
        void handle_joints();

    public:
        // Public variables
        Time physicsTimestep = Time::seconds(-1); // If -1, use app's timestep.
        World world{{0.f, 0.f}};

        // Constructors
        PhysicsSystem(Scene& sceneRef);
        PhysicsSystem(PhysicsSystem&& other) = delete;
        PhysicsSystem(const PhysicsSystem& other) = delete;
        ~PhysicsSystem();

        // Operators
        PhysicsSystem& operator=(PhysicsSystem&& other) = delete;
        PhysicsSystem& operator=(const PhysicsSystem& other) = delete;

        // Functions
        void update();
    };
};