#include <gtest/gtest.h>
#include "draft/components/camera_component.hpp"
#include "draft/util/serialization/serializer.hpp"

using namespace Draft;

// Camera has its own explicit serialize/deserialize (it needs to recompute derived matrices
// after loading), and CameraComponent is a plain DRAFT_REFLECTABLE aggregate over
static_assert(Serializer::JsonSerializable<Camera>);
static_assert(Serializer::BinarySerializable<Camera>);
static_assert(Reflectable<CameraComponent>);
static_assert(!Serializer::JsonSerializable<CameraComponent>);

TEST(CameraComponent, JsonRoundTripReachesTheUnderlyingCamera)
{
    CameraComponent original{true, 3, Camera::make_orthographic({1, 2, 3}, {0, 0, -1}, -10.f, 10.f, -5.f, 5.f)};

    JSON json;
    Serializer::serialize(original, json);

    CameraComponent restored;
    Serializer::deserialize(restored, json);

    EXPECT_EQ(restored.active, true);
    EXPECT_EQ(restored.priority, 3);
    EXPECT_EQ(restored.camera.get_type(), CameraType::ORTHOGRAPHIC);
    EXPECT_FLOAT_EQ(restored.camera.get_position().x, 1.f);
    EXPECT_FLOAT_EQ(restored.camera.get_orthographic_params().rightClip, 10.f);
}

// Regression test: CameraComponent's Binary encoding goes through the reflect tier's
// deserialize_and_advance, which needs Camera's own deserialize_and_advance
TEST(CameraComponent, BinaryRoundTripOfTwoComponentsInARow)
{
    CameraComponent first{true, 0, Camera::make_orthographic({0, 0, 0}, {0, 0, -1}, -1.f, 1.f, -1.f, 1.f)};
    CameraComponent second{true, 5, Camera::make_perspective({0, 0, 0}, {0, 0, -1}, 2.f, 60.f)};

    Binary::ByteArray out;
    Serializer::serialize(first, out);
    Serializer::serialize(second, out);

    Binary::ByteView span(out);
    CameraComponent restoredFirst, restoredSecond;
    Serializer::deserialize_and_advance(restoredFirst, span);
    Serializer::deserialize_and_advance(restoredSecond, span);

    EXPECT_EQ(restoredFirst.camera.get_type(), CameraType::ORTHOGRAPHIC);

    EXPECT_EQ(restoredSecond.priority, 5);
    EXPECT_EQ(restoredSecond.camera.get_type(), CameraType::PERSPECTIVE);
    EXPECT_FLOAT_EQ(restoredSecond.camera.get_perspective_params().fov, 60.f);
}
