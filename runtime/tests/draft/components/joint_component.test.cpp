#include <gtest/gtest.h>
#include "draft/components/joint_component.hpp"
#include <set>
#include <string>

using namespace Draft;

TEST(NativeJointComponent, DefaultConstructionIsInvalid)
{
    NativeJointComponent<DistanceJointData> native;

    EXPECT_EQ(native.jointPtr, nullptr);
    EXPECT_FALSE(native.is_valid());
    EXPECT_FALSE(static_cast<bool>(native));
    EXPECT_FLOAT_EQ(native.delta.length, 1.f); // Inherited DistanceJointData default
}

TEST(JointComponent, DefaultConstructionHasInvalidEntitiesAndNoHandle)
{
    DistanceJointComponent joint;

    EXPECT_FALSE(joint.entityA.is_valid());
    EXPECT_FALSE(joint.entityB.is_valid());
    EXPECT_EQ(joint.m_nativeHandlePtr, nullptr);
}

TEST(JointComponent, AccessorsWithNoNativeHandleReturnSafeDefaults)
{
    DistanceJointComponent joint;

    EXPECT_EQ(joint.get_type(), Joint::Type::UNKNOWN);
    EXPECT_FLOAT_EQ(joint.get_world_anchor_a().x, 0.f);
    EXPECT_FALSE(joint.get_collide_connected());
    EXPECT_FALSE(joint.is_enabled());
}

TEST(ConstrainedComponent, DefaultConstructionHasNoConstraints)
{
    ConstrainedComponent constrained;
    EXPECT_TRUE(constrained.constraints.empty());
}

TEST(JointComponentTypes, DraftAllJointTypesListsAllTenTypes)
{
    // Matches Joint::Type minus UNKNOWN/ROPE, neither of which has a concrete Joint subclass.
    EXPECT_EQ(std::tuple_size<JointComponentTypes>::value, 10u);
}

TEST(JointComponentReflection, EachTypeHasItsOwnDistinctReflectName)
{
    std::set<std::string_view> names = {
        DistanceJointComponent::reflect_name(),
        FrictionJointComponent::reflect_name(),
        GearJointComponent::reflect_name(),
        MotorJointComponent::reflect_name(),
        MouseJointComponent::reflect_name(),
        PrismaticJointComponent::reflect_name(),
        PulleyJointComponent::reflect_name(),
        RevoluteJointComponent::reflect_name(),
        WeldJointComponent::reflect_name(),
        WheelJointComponent::reflect_name(),
    };

    EXPECT_EQ(DistanceJointComponent::reflect_name(), "DistanceJointComponent");
    EXPECT_EQ(names.size(), 10u); // would collapse to 1 if they all still reported "JointComponent"
}

TEST(JointComponentReflection, ForEachFieldVisitsOwnFieldsNotJustEntities)
{
    DistanceJointComponent joint;
    joint.length = 5.f;

    std::set<std::string_view> visited;
    for_each_field(joint, [&](std::string_view name, const auto&){ visited.insert(name); });

    EXPECT_TRUE(visited.contains("entityA"));
    EXPECT_TRUE(visited.contains("entityB"));
    EXPECT_TRUE(visited.contains("collideConnected"));
    EXPECT_TRUE(visited.contains("length"));
    EXPECT_TRUE(visited.contains("stiffness"));
}

TEST(CreateJointEntity, CreatesAnEntityWithTheGivenJointData)
{
    Scene scene;
    DistanceJointData data;
    data.length = 5.f;

    DistanceJointComponent component;
    static_cast<DistanceJointData&>(component) = data;

    auto [entity, ref] = create_joint_entity(scene, component);

    EXPECT_TRUE(entity.is_valid());
    EXPECT_FLOAT_EQ(ref.length, 5.f);
    EXPECT_TRUE(entity.has_component<DistanceJointComponent>());
}
