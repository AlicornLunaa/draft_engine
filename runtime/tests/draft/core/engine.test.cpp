#include <gtest/gtest.h>
#include "draft/core/engine.hpp"
#include "draft/components/tag_component.hpp"
#include "draft/components/transform_component.hpp"

using namespace Draft;

namespace {
    struct Health {
        DRAFT_REFLECTED(int, value) = 0;

        DRAFT_REFLECTABLE(Health, value)
    };
}

TEST(Engine, DefaultConstructsWithTransformAndTagRegistered)
{
    Engine engine;

    ASSERT_NE(engine.components().by_type<TransformComponent>(), nullptr);
    ASSERT_EQ(engine.components().by_type<TransformComponent>()->name(), "TransformComponent");

    ASSERT_NE(engine.components().by_type<TagComponent>(), nullptr);
    ASSERT_EQ(engine.components().by_type<TagComponent>()->name(), "TagComponent");
}

TEST(Engine, RegisterComponentForwardsToTheComponentCatalog)
{
    Engine engine;
    engine.register_component<Health>();

    ASSERT_NE(engine.components().by_name("Health"), nullptr);
    ASSERT_EQ(engine.components().by_name("Health"), engine.components().by_type<Health>());
}
