#pragma once

#include "draft/core/entity.hpp"
#include "draft/phys/joint_def.hpp"
#include <memory>

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

    // Factory
    template<typename T>
    Entity create_joint_entity(Scene& scene, const T& joint){
        Entity entity = scene.create_entity();
        entity.add_component<T>(joint);
        return entity;
    }
};