#include <gtest/gtest.h>
#include "draft/phys/world.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/shapes/circle_shape.hpp"
#include "draft/phys/fixture_def.hpp"

TEST(World, Gravity)
{
    Draft::World world({0.f, -9.8f});
    Draft::BodyDef def;
    def.type = Draft::RigidBody::BodyType::DYNAMIC;
    def.position = {0.f, 0.f};

    Draft::RigidBody* body = world.create_rigid_body(def);
    ASSERT_NE(body, nullptr);

    Draft::CircleShape shape;
    shape.set_radius(1.f);
    
    Draft::FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 1.f;
    body->create_fixture(fixtureDef);

    for(int i = 0; i < 60; ++i)
        world.step(Draft::Time::seconds(1.f/60.f), 8, 3);
    
    Draft::Vector2f pos = body->get_position();
    ASSERT_NEAR(pos.y, -4.9f, 0.1f);
}

TEST(World, CreateDestroy)
{
    Draft::World world({0.f, 0.f});
    ASSERT_EQ(world.get_body_count(), 0);

    Draft::BodyDef def;
    def.type = Draft::RigidBody::BodyType::DYNAMIC;
    
    Draft::RigidBody* body = world.create_rigid_body(def);
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(world.get_body_count(), 1);

    world.destroy_body(body);
    ASSERT_EQ(world.get_body_count(), 0);
}

TEST(World, BodyType)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef def;
    def.type = Draft::RigidBody::BodyType::DYNAMIC;
    
    Draft::RigidBody* body = world.create_rigid_body(def);
    ASSERT_EQ(body->get_type(), Draft::RigidBody::BodyType::DYNAMIC);
}
