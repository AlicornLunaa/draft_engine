#include "draft/systems.h"
#include "draft/util/constants.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/rigid_body_component.hpp"

void Draft::physicsSystem(entt::registry& registry, World& world){
    world.step(TIME_STEP, VELOCITY_ITER, POSITION_ITER);

    auto view = registry.view<TransformComponent, RigidBodyComponent>();
    for(auto entity : view){
        auto& transformComponent = view.get<TransformComponent>(entity);
        auto& rigidBodyComponent = view.get<RigidBodyComponent>(entity);

        Transform trans;
        trans.translate(rigidBodyComponent.getPosition());
        trans.rotate(TO_DEG(rigidBodyComponent.getAngle()));
        transformComponent.transform = trans;
    }
}