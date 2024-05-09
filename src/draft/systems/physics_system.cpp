#include "draft/core/application.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/phys/fixture.hpp"
#include "draft/systems.hpp"

void Draft::physics_system(Registry& registry, const Application* app, World& world){
    world.step(app->timeStep, world.VELOCITY_ITER, world.POSITION_ITER);

    auto view = registry.view<TransformComponent, RigidBodyComponent>();

    for(auto entity : view){
        TransformComponent& transformComponent = view.get<TransformComponent>(entity);
        RigidBody& rigidBodyComponent = view.get<RigidBodyComponent>(entity);

        transformComponent.position = rigidBodyComponent.get_position();
        transformComponent.rotation = rigidBodyComponent.get_angle();
    }
}