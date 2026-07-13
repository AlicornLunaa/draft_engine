#include <gtest/gtest.h>
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/util/serialization/context.hpp"
#include "draft/util/serialization/serializer.hpp"

#include <cstdint>

using namespace Draft;

static_assert(Serializer::Serializable<Entity>);

namespace {
    struct EntitySerializerTest : ::testing::Test {
        Scene scene;
        Entity entity = scene.create_entity();
        SceneSerializationContext ctx{
            nullptr,
            {{static_cast<entt::entity>(entity), 0}},
            {entity}
        };
    };
}

TEST_F(EntitySerializerTest, JsonRoundTripResolvesToTheSameEntity)
{
    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

    JSON json;
    Serializer::serialize(entity, json);
    ASSERT_EQ(json.get<uint32_t>(), 0u);

    Entity restored;
    Serializer::deserialize(restored, json);
    ASSERT_EQ(restored, entity);
}

TEST_F(EntitySerializerTest, BinaryRoundTripResolvesToTheSameEntity)
{
    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

    Binary::ByteArray buffer;
    Serializer::serialize(entity, buffer);

    Entity restored;
    Serializer::deserialize(restored, Binary::ByteView(buffer));
    ASSERT_EQ(restored, entity);
}

TEST_F(EntitySerializerTest, NullEntityJsonRoundTripsToNullEntity)
{
    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

    JSON json;
    Serializer::serialize(NULL_ENTITY, json);
    ASSERT_EQ(json.get<uint32_t>(), UINT32_MAX);

    Entity restored = entity; // starts non-null, deserialize should overwrite it
    Serializer::deserialize(restored, json);
    ASSERT_EQ(restored, NULL_ENTITY);
}

TEST_F(EntitySerializerTest, NullEntityBinaryRoundTripsToNullEntity)
{
    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

    Binary::ByteArray buffer;
    Serializer::serialize(NULL_ENTITY, buffer);

    Entity restored = entity;
    Serializer::deserialize(restored, Binary::ByteView(buffer));
    ASSERT_EQ(restored, NULL_ENTITY);
}
