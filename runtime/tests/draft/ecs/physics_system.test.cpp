#include <gtest/gtest.h>
#include "draft/ecs/physics_system.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/components/joint_component.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/phys/shapes/circle_shape.hpp"
#include "draft/phys/world.hpp"

using namespace Draft;

TEST(PhysicsSystem, ConstructingRigidBodyComponentCreatesAValidNativeHandle)
{
    World world({0.f, 0.f});
    Scene scene;
    scene.get_systems().add<PhysicsSystem>(scene, world);

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>(TransformComponent{{1.f, 2.f}, 0.f});
    entity.add_component<RigidBodyComponent>(RigidBodyComponent{.type = BodyType::DYNAMIC});

    ASSERT_TRUE(entity.has_component<NativeBodyComponent>());
    NativeBodyComponent& native = entity.get_component<NativeBodyComponent>();
    ASSERT_TRUE(native.is_valid());
    EXPECT_FLOAT_EQ(native.bodyPtr->get_position().x, 1.f);
    EXPECT_FLOAT_EQ(native.bodyPtr->get_position().y, 2.f);
}

TEST(PhysicsSystem, DestroyingRigidBodyComponentRemovesTheNativeHandle)
{
    World world({0.f, 0.f});
    Scene scene;
    scene.get_systems().add<PhysicsSystem>(scene, world);

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>();
    entity.add_component<RigidBodyComponent>();
    ASSERT_TRUE(entity.has_component<NativeBodyComponent>());
    ASSERT_EQ(world.get_body_count(), 1u);

    entity.remove_component<RigidBodyComponent>();

    EXPECT_FALSE(entity.has_component<NativeBodyComponent>());
    EXPECT_EQ(world.get_body_count(), 0u);
}

TEST(PhysicsSystem, ColliderComponentAttachesOnceABodyExists)
{
    World world({0.f, 0.f});
    Scene scene;
    scene.get_systems().add<PhysicsSystem>(scene, world);

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>();

    CircleShape circle;
    ColliderComponent& colliderComp = entity.add_component<ColliderComponent>(ColliderComponent(circle));
    EXPECT_FALSE(colliderComp.collider.is_attached()); // No body yet

    entity.add_component<RigidBodyComponent>();

    EXPECT_TRUE(colliderComp.collider.is_attached());
    RigidBody* body = entity.get_component<NativeBodyComponent>();
    EXPECT_EQ(body->get_fixture_list().size(), 1u);
}

TEST(PhysicsSystem, DestroyingTheEntityDetachesTheColliderWithoutDangling)
{
    World world({0.f, 0.f});
    Scene scene;
    scene.get_systems().add<PhysicsSystem>(scene, world);

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>();
    entity.add_component<RigidBodyComponent>();

    CircleShape circle;
    entity.add_component<ColliderComponent>(ColliderComponent(circle));
    ASSERT_EQ(world.get_body_count(), 1u);

    entity.destroy();

    EXPECT_EQ(world.get_body_count(), 0u);
}

TEST(PhysicsSystem, JointComponentPairCreatesALiveJointAndConstrainsBothEntities)
{
    World world({0.f, 0.f});
    Scene scene;
    scene.get_systems().add<PhysicsSystem>(scene, world);

    Entity bodyA = scene.create_entity();
    bodyA.add_component<TransformComponent>();
    bodyA.add_component<RigidBodyComponent>(RigidBodyComponent{.type = BodyType::DYNAMIC});

    Entity bodyB = scene.create_entity();
    bodyB.add_component<TransformComponent>(TransformComponent{{2.f, 0.f}, 0.f});
    bodyB.add_component<RigidBodyComponent>(RigidBodyComponent{.type = BodyType::DYNAMIC});

    Entity jointEntity = scene.create_entity();
    DistanceJointComponent jointData;
    jointData.entityA = bodyA;
    jointData.entityB = bodyB;
    jointData.length = 2.f;
    jointEntity.add_component<DistanceJointComponent>(jointData);

    ASSERT_TRUE(jointEntity.has_component<DistanceJointComponent::NativeType>());
    DistanceJointComponent::NativeType& native = jointEntity.get_component<DistanceJointComponent::NativeType>();
    ASSERT_TRUE(native.is_valid());
    EXPECT_EQ(native.get_as<DistanceJoint>()->get_length(), 2.f);

    ASSERT_TRUE(bodyA.has_component<ConstrainedComponent>());
    EXPECT_EQ(bodyA.get_component<ConstrainedComponent>().constraints[0], jointEntity);
    ASSERT_TRUE(bodyB.has_component<ConstrainedComponent>());
    EXPECT_EQ(bodyB.get_component<ConstrainedComponent>().constraints[0], jointEntity);

    jointEntity.destroy();

    EXPECT_FALSE(bodyA.has_component<ConstrainedComponent>());
    EXPECT_FALSE(bodyB.has_component<ConstrainedComponent>());
}

TEST(PhysicsSystem, OneShotTorqueComponentIsConsumedAfterOneUpdate)
{
    World world({0.f, 0.f});
    Scene scene;
    scene.get_systems().add<PhysicsSystem>(scene, world);

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>();
    entity.add_component<RigidBodyComponent>(RigidBodyComponent{.type = BodyType::DYNAMIC});
    entity.add_component<TorqueComponent>(TorqueComponent{1.f, true});

    scene.update(Time::seconds(1.f / 60.f));

    EXPECT_FALSE(entity.has_component<TorqueComponent>());
}

TEST(PhysicsSystem, ContinuousForceComponentIsNotConsumed)
{
    World world({0.f, 0.f});
    Scene scene;
    scene.get_systems().add<PhysicsSystem>(scene, world);

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>();
    entity.add_component<RigidBodyComponent>(RigidBodyComponent{.type = BodyType::DYNAMIC});
    entity.add_component<ContinuousForceComponent>(ContinuousForceComponent{{1.f, 0.f}, {0.f, 0.f}, true, true});

    scene.update(Time::seconds(1.f / 60.f));

    EXPECT_TRUE(entity.has_component<ContinuousForceComponent>());
}

TEST(PhysicsSystem, ModifyingTransformForceSyncsTheNativeBodyImmediately)
{
    World world({0.f, 0.f});
    Scene scene;
    scene.get_systems().add<PhysicsSystem>(scene, world);

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>();
    entity.add_component<RigidBodyComponent>(RigidBodyComponent{.type = BodyType::DYNAMIC});

    entity.modify_component<TransformComponent>([](TransformComponent& t){ t.position = {5.f, 6.f}; });

    RigidBody* body = entity.get_component<NativeBodyComponent>();
    EXPECT_FLOAT_EQ(body->get_position().x, 5.f);
    EXPECT_FLOAT_EQ(body->get_position().y, 6.f);
}
