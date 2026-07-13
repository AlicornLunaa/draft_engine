#include <gtest/gtest.h>
#include "draft/util/serialization/serializer.hpp"

using namespace Draft;

static_assert(Serializer::CustomJsonSerializable<Vector2f>);
static_assert(Serializer::CustomJsonSerializable<Vector3i>);
static_assert(!Reflectable<Vector2f>);
static_assert(!Serializer::JsonSerializable<Vector2f>);

TEST(GlmSerializer, Vector2JsonRoundTrip)
{
    Vector2f original{1.5f, -2.5f};

    JSON json;
    Serializer::serialize(original, json);

    Vector2f restored;
    Serializer::deserialize(restored, json);

    ASSERT_FLOAT_EQ(restored.x, 1.5f);
    ASSERT_FLOAT_EQ(restored.y, -2.5f);
}

TEST(GlmSerializer, Vector3JsonRoundTrip)
{
    Vector3i original{1, 2, 3};

    JSON json;
    Serializer::serialize(original, json);

    Vector3i restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored.x, 1);
    ASSERT_EQ(restored.y, 2);
    ASSERT_EQ(restored.z, 3);
}

TEST(GlmSerializer, Vector4JsonRoundTrip)
{
    Vector4f original{1.f, 2.f, 3.f, 4.f};

    JSON json;
    Serializer::serialize(original, json);

    Vector4f restored;
    Serializer::deserialize(restored, json);

    ASSERT_FLOAT_EQ(restored.x, 1.f);
    ASSERT_FLOAT_EQ(restored.y, 2.f);
    ASSERT_FLOAT_EQ(restored.z, 3.f);
    ASSERT_FLOAT_EQ(restored.w, 4.f);
}

TEST(GlmSerializer, Vector2BinaryRoundTripViaTrivialFallback)
{
    // Not part of the CustomSerializer specialization, proves Binary keeps working through the
    // ordinary TriviallySerializable tier, since vec<L, T, Q> is plain POD.
    Vector2f original{3.f, 4.f};

    Binary::ByteArray buffer;
    Serializer::serialize(original, buffer);

    Binary::ByteView view(buffer);
    Vector2f restored;
    Serializer::deserialize(restored, view);

    ASSERT_FLOAT_EQ(restored.x, 3.f);
    ASSERT_FLOAT_EQ(restored.y, 4.f);
}
