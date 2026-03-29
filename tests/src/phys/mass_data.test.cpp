#include <gtest/gtest.h>
#include "draft/phys/mass_data.hpp"

TEST(MassData, DefaultConstructor)
{
    Draft::MassData data;
    ASSERT_EQ(data.mass, 0.f);
    ASSERT_EQ(data.centerOfMass.x, 0.f);
    ASSERT_EQ(data.centerOfMass.y, 0.f);
    ASSERT_EQ(data.inertia, 0.f);
}

TEST(MassData, CustomValues)
{
    Draft::MassData data;
    data.mass = 10.f;
    data.centerOfMass = {1.f, 2.f};
    data.inertia = 5.f;

    ASSERT_EQ(data.mass, 10.f);
    ASSERT_EQ(data.centerOfMass.x, 1.f);
    ASSERT_EQ(data.centerOfMass.y, 2.f);
    ASSERT_EQ(data.inertia, 5.f);
}
