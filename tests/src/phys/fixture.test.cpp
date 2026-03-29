#include <gtest/gtest.h>
#include "draft/phys/fixture.hpp"
#include "draft/phys/world.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/body_def.hpp"
#include "draft/phys/fixture_def.hpp"
#include "draft/phys/shapes/circle_shape.hpp"

TEST(Fixture, CreationAndDestruction)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    Draft::CircleShape circleShape;
    Draft::FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;

    Draft::Fixture* fixture = body->create_fixture(fixtureDef);
    ASSERT_NE(fixture, nullptr);
    ASSERT_TRUE(fixture->is_valid());
    ASSERT_EQ(fixture->get_body(), body);
    ASSERT_EQ(fixture->get_shape(), &circleShape);
    ASSERT_EQ(fixture->get_density(), 1.0f);

    body->destroy_fixture(fixture);
    world.destroy_body(body);
}

TEST(Fixture, SetGetDensity)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    Draft::CircleShape circleShape;
    Draft::FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;

    Draft::Fixture* fixture = body->create_fixture(fixtureDef);
    ASSERT_NE(fixture, nullptr);

    fixture->set_density(2.0f);
    ASSERT_EQ(fixture->get_density(), 2.0f);

    world.destroy_body(body);
}

TEST(Fixture, SetGetFriction)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    Draft::CircleShape circleShape;
    Draft::FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.friction = 0.5f;

    Draft::Fixture* fixture = body->create_fixture(fixtureDef);
    ASSERT_NE(fixture, nullptr);

    fixture->set_friction(0.7f);
    ASSERT_EQ(fixture->get_friction(), 0.7f);

    world.destroy_body(body);
}

TEST(Fixture, SetGetRestitution)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    Draft::CircleShape circleShape;
    Draft::FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.restitution = 0.3f;

    Draft::Fixture* fixture = body->create_fixture(fixtureDef);
    ASSERT_NE(fixture, nullptr);

    fixture->set_restitution(0.9f);
    ASSERT_EQ(fixture->get_restitution(), 0.9f);

    world.destroy_body(body);
}

TEST(Fixture, SetGetRestitutionThreshold)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    Draft::CircleShape circleShape;
    Draft::FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.restitutionThreshold = 0.5f;

    Draft::Fixture* fixture = body->create_fixture(fixtureDef);
    ASSERT_NE(fixture, nullptr);

    fixture->set_restitution_threshold(0.8f);
    ASSERT_EQ(fixture->get_restitution_threshold(), 0.8f);

    world.destroy_body(body);
}

TEST(Fixture, SetGetSensor)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef bodyDef;
    Draft::RigidBody* body = world.create_rigid_body(bodyDef);

    Draft::CircleShape circleShape;
    Draft::FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.isSensor = false;

    Draft::Fixture* fixture = body->create_fixture(fixtureDef);
    ASSERT_NE(fixture, nullptr);
    ASSERT_FALSE(fixture->is_sensor());

    fixture->set_sensor(true);
    ASSERT_TRUE(fixture->is_sensor());

    fixture->set_sensor(false);
    ASSERT_FALSE(fixture->is_sensor());

    world.destroy_body(body);
}