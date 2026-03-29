#include <gtest/gtest.h>
#include "draft/phys/world.hpp"
#include "draft/phys/rigid_body.hpp"
#include "draft/phys/body_def.hpp"

TEST(RigidBody, CreateAndDestroy)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef def;
    def.type = Draft::RigidBody::BodyType::STATIC;
    def.position = {1.0f, 2.0f};
    def.angle = 0.5f;

    Draft::RigidBody* body = world.create_rigid_body(def);
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->get_type(), Draft::RigidBody::BodyType::STATIC);
    ASSERT_EQ(body->get_position().x, 1.0f);
    ASSERT_EQ(body->get_position().y, 2.0f);
    ASSERT_EQ(body->get_angle(), 0.5f);

    world.destroy_body(body);
}

TEST(RigidBody, SetPositionAndAngle)
{
    Draft::World world({0.f, 0.f});
    Draft::BodyDef def;
    Draft::RigidBody* body = world.create_rigid_body(def);

    body->set_transform({5.0f, 6.0f}, 1.5f);

    ASSERT_EQ(body->get_position().x, 5.0f);
    ASSERT_EQ(body->get_position().y, 6.0f);
    ASSERT_EQ(body->get_angle(), 1.5f);

    world.destroy_body(body);
}
