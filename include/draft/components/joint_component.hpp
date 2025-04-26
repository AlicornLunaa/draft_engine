#pragma once

#include "draft/core/entity.hpp"
#include "draft/phys/joint_def.hpp"
#include <memory>
#include <vector>

namespace Draft {
    // Components
    struct JointComponent {
        Entity entityA;
        Entity entityB;
        std::unique_ptr<GenericJointData> definition;
    };

    struct NativeJointComponent {
        Joint* joint = nullptr;
        Joint* operator->(){ return joint; }
    };

    struct ConstrainedComponent {
        // Holds a list of all the joint components which constraint the entitiy to another entitiy
        std::vector<Entity> constraints;
    };

    // Factory
    template<typename T>
    Entity create_joint_entity(Scene& scene, Entity a, Entity b, const T& data){
        Entity entity = scene.create_entity();
        entity.add_component<JointComponent>(a, b, std::make_unique<T>(data));
        return entity;
    }
};