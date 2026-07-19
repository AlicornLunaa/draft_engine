#include <gtest/gtest.h>
#include "draft/util/serialization/serializer.hpp"
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

using namespace Draft;
using namespace Draft::Binary;

TEST(Binary, LittleEndianRoundTrip)
{
    int32_t value = 0x11223344;
    int32_t restored = from_little_endian(to_little_endian(value));
    ASSERT_EQ(restored, value);
}

TEST(Binary, WriteAndReadTrivialTypes)
{
    ByteArray buffer;
    write(buffer, int32_t{123456});
    write(buffer, 3.5f);

    ByteView view(buffer);

    int32_t i = 0;
    read_and_advance(view, i);
    ASSERT_EQ(i, 123456);

    float f = 0.f;
    read_and_advance(view, f);
    ASSERT_FLOAT_EQ(f, 3.5f);

    ASSERT_TRUE(view.empty());
}

TEST(Binary, ReadThrowsWhenSpanTooSmall)
{
    ByteArray buffer;
    write(buffer, int16_t{1}); // only 2 bytes available

    ByteView view(buffer);
    int64_t value = 0;
    ASSERT_THROW(read(view, value), std::runtime_error);
}

TEST(SerializerTrivial, BinaryRoundTrip)
{
    ByteArray buffer;
    Serializer::serialize(42, buffer);

    ByteView view(buffer);
    int restored = 0;
    Serializer::deserialize(restored, view);
    ASSERT_EQ(restored, 42);
}

TEST(SerializerTrivial, JsonRoundTrip)
{
    JSON json;
    Serializer::serialize(std::string("hello"), json);

    std::string restored;
    Serializer::deserialize(restored, json);
    ASSERT_EQ(restored, "hello");
}

TEST(SerializerVector, BinaryRoundTrip)
{
    std::vector<int> values = {1, 2, 3, 4, 5};

    ByteArray buffer;
    Serializer::serialize(values, buffer);

    ByteView view(buffer);
    std::vector<int> restored;
    Serializer::deserialize(restored, view);

    ASSERT_EQ(restored, values);
}

namespace {
    // A custom type with its own static serialize/deserialize, satisfying both
    // BinarySerializable and JsonSerializable. Deliberately non-trivially-copyable (owns a
    // std::string) so it can't also match Serializer's built-in "trivial type" overloads
    //
    // Its binary encoding is variable-length (the string's length varies), so unlike the
    // vector test above, this type must not be run through deserialize_and_advance().
    struct Label {
        std::string name;
        int value = 0;

        static void serialize(const Label& l, Binary::ByteArray& out) {
            Binary::write(out, l.name.size());
            out.insert(out.end(),
                reinterpret_cast<const std::byte*>(l.name.data()),
                reinterpret_cast<const std::byte*>(l.name.data()) + l.name.size());
            Binary::write(out, l.value);
        }

        static void deserialize(Label& l, Binary::ByteView span) {
            size_t size = 0;
            Binary::read(span, size);
            span = span.subspan(sizeof(size));

            l.name.assign(reinterpret_cast<const char*>(span.data()), size);
            span = span.subspan(size);

            Binary::read(span, l.value);
        }

        static void serialize(const Label& l, JSON& json) {
            json["name"] = l.name;
            json["value"] = l.value;
        }

        static void deserialize(Label& l, const JSON& json) {
            l.name = json.at("name").get<std::string>();
            l.value = json.at("value").get<int>();
        }
    };

    static_assert(Serializer::BinarySerializable<Label>);
    static_assert(Serializer::JsonSerializable<Label>);
    static_assert(Serializer::Serializable<Label>);
}

TEST(SerializerCustomType, BinaryRoundTrip)
{
    Label original{"score", 42};

    Binary::ByteArray buffer;
    Serializer::serialize(original, buffer);

    Binary::ByteView view(buffer);
    Label restored;
    Serializer::deserialize(restored, view);

    ASSERT_EQ(restored.name, "score");
    ASSERT_EQ(restored.value, 42);
}

TEST(SerializerCustomType, JsonRoundTrip)
{
    Label original{"health", 100};

    JSON json;
    Serializer::serialize(original, json);

    Label restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored.name, "health");
    ASSERT_EQ(restored.value, 100);
}

namespace {
    // Plain reflectable struct with no explicit serialize/deserialize of its own, must be
    // picked up by the reflect tier, not the trivial tier's overload (even though it happens to
    // also be trivially copyable).
    struct Point {
        DRAFT_REFLECTED(int, x) = 0;
        DRAFT_REFLECTED(int, y) = 0;

        DRAFT_REFLECTABLE(Point, x, y)
    };

    // A reflectable struct with a variable-length field (a vector), forcing the reflect tier's
    // binary encoding to actually advance between fields instead of assuming sizeof(T).
    struct Path {
        DRAFT_REFLECTED(std::string, name);
        DRAFT_REFLECTED(std::vector<int>, waypoints);

        DRAFT_REFLECTABLE(Path, name, waypoints)
    };

    // A reflectable struct nesting another reflectable struct, to exercise recursive dispatch.
    struct Line {
        DRAFT_REFLECTED(Point, start);
        DRAFT_REFLECTED(Point, end);

        DRAFT_REFLECTABLE(Line, start, end)
    };

    static_assert(Reflectable<Point>);
    static_assert(!Serializer::BinarySerializable<Point>);
    static_assert(!Serializer::JsonSerializable<Point>);
}

TEST(SerializerReflectable, BinaryRoundTrip)
{
    Point original{3, 4};

    Binary::ByteArray buffer;
    Serializer::serialize(original, buffer);

    Binary::ByteView view(buffer);
    Point restored;
    Serializer::deserialize(restored, view);

    ASSERT_EQ(restored.x, 3);
    ASSERT_EQ(restored.y, 4);
}

TEST(SerializerReflectable, JsonRoundTrip)
{
    Point original{5, 6};

    JSON json;
    Serializer::serialize(original, json);

    Point restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored.x, 5);
    ASSERT_EQ(restored.y, 6);
}

TEST(SerializerReflectable, BinaryRoundTripVariableLength)
{
    Path original{"trail", {1, 2, 3, 4}};

    Binary::ByteArray buffer;
    Serializer::serialize(original, buffer);

    // Two Paths back-to-back must not overlap, proves fields advance the span correctly.
    Serializer::serialize(original, buffer);

    Binary::ByteView view(buffer);
    Path first, second;
    Serializer::deserialize_and_advance(first, view);
    Serializer::deserialize_and_advance(second, view);

    ASSERT_EQ(first.name, "trail");
    ASSERT_EQ(first.waypoints, (std::vector<int>{1, 2, 3, 4}));
    ASSERT_EQ(second.name, "trail");
    ASSERT_EQ(second.waypoints, (std::vector<int>{1, 2, 3, 4}));
    ASSERT_TRUE(view.empty());
}

TEST(SerializerReflectable, JsonRoundTripVariableLength)
{
    Path original{"trail", {1, 2, 3, 4}};

    JSON json;
    Serializer::serialize(original, json);

    Path restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored.name, "trail");
    ASSERT_EQ(restored.waypoints, (std::vector<int>{1, 2, 3, 4}));
}

TEST(SerializerReflectable, BinaryRoundTripNested)
{
    Line original{{0, 0}, {10, 20}};

    Binary::ByteArray buffer;
    Serializer::serialize(original, buffer);

    Binary::ByteView view(buffer);
    Line restored;
    Serializer::deserialize(restored, view);

    ASSERT_EQ(restored.start.x, 0);
    ASSERT_EQ(restored.start.y, 0);
    ASSERT_EQ(restored.end.x, 10);
    ASSERT_EQ(restored.end.y, 20);
}

TEST(SerializerReflectable, JsonRoundTripNested)
{
    Line original{{0, 0}, {10, 20}};

    JSON json;
    Serializer::serialize(original, json);

    Line restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored.start.x, 0);
    ASSERT_EQ(restored.start.y, 0);
    ASSERT_EQ(restored.end.x, 10);
    ASSERT_EQ(restored.end.y, 20);
}

TEST(SerializerVector, JsonRoundTrip)
{
    std::vector<int> values = {1, 2, 3, 4, 5};

    JSON json;
    Serializer::serialize(values, json);

    std::vector<int> restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored, values);
}

TEST(SerializerVariant, BinaryRoundTripPreservesActiveAlternative)
{
    // Starts on the variant's default alternative (index 0, int), the encoded index must switch
    // `restored` over to Point before deserializing its fields.
    std::variant<int, Point> value = Point{3, 4};

    Binary::ByteArray buffer;
    Serializer::serialize(value, buffer);

    Binary::ByteView view(buffer);
    std::variant<int, Point> restored;
    Serializer::deserialize(restored, view);

    ASSERT_EQ(restored.index(), 1u);
    ASSERT_EQ(std::get<Point>(restored).x, 3);
    ASSERT_EQ(std::get<Point>(restored).y, 4);
}

TEST(SerializerVariant, BinaryRoundTripSwitchesAwayFromTheOtherAlternative)
{
    // `restored` starts on Point (the opposite of the previous test), exercising the switch in
    // the other direction.
    std::variant<int, Point> value = 42;

    Binary::ByteArray buffer;
    Serializer::serialize(value, buffer);

    Binary::ByteView view(buffer);
    std::variant<int, Point> restored{Point{}};
    Serializer::deserialize(restored, view);

    ASSERT_EQ(restored.index(), 0u);
    ASSERT_EQ(std::get<int>(restored), 42);
}

TEST(SerializerVariant, JsonRoundTrip)
{
    std::variant<int, Point> value = Point{5, 6};

    JSON json;
    Serializer::serialize(value, json);

    std::variant<int, Point> restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored.index(), 1u);
    ASSERT_EQ(std::get<Point>(restored).x, 5);
    ASSERT_EQ(std::get<Point>(restored).y, 6);
}

TEST(SerializerOptional, BinaryRoundTripWithValue)
{
    std::optional<Point> value = Point{3, 4};

    Binary::ByteArray buffer;
    Serializer::serialize(value, buffer);

    Binary::ByteView view(buffer);
    std::optional<Point> restored;
    Serializer::deserialize(restored, view);

    ASSERT_TRUE(restored.has_value());
    ASSERT_EQ(restored->x, 3);
    ASSERT_EQ(restored->y, 4);
}

TEST(SerializerOptional, BinaryRoundTripWithNoValueClearsAnExistingOne)
{
    std::optional<Point> value;

    Binary::ByteArray buffer;
    Serializer::serialize(value, buffer);

    Binary::ByteView view(buffer);
    std::optional<Point> restored = Point{1, 2}; // pre-existing value should be cleared
    Serializer::deserialize(restored, view);

    ASSERT_FALSE(restored.has_value());
}

TEST(SerializerOptional, JsonRoundTripWithValue)
{
    std::optional<Point> value = Point{5, 6};

    JSON json;
    Serializer::serialize(value, json);

    std::optional<Point> restored;
    Serializer::deserialize(restored, json);

    ASSERT_TRUE(restored.has_value());
    ASSERT_EQ(restored->x, 5);
    ASSERT_EQ(restored->y, 6);
}

TEST(SerializerOptional, JsonRoundTripWithNoValueSerializesToNull)
{
    std::optional<Point> value;

    JSON json;
    Serializer::serialize(value, json);
    ASSERT_TRUE(json.is_null());

    std::optional<Point> restored = Point{1, 2};
    Serializer::deserialize(restored, json);

    ASSERT_FALSE(restored.has_value());
}

namespace {
    // Stands in for a type we don't own (e.g. a third-party math type)
    struct ForeignPoint {
        int x = 0;
        int y = 0;
    };
}

template<>
struct Draft::Serializer::CustomSerializer<ForeignPoint> {
    static void serialize(const ForeignPoint& p, Binary::ByteArray& out){
        Serializer::serialize(p.x, out);
        Serializer::serialize(p.y, out);
    }

    static void deserialize(ForeignPoint& p, Binary::ByteView span){
        Serializer::deserialize_and_advance(p.x, span);
        Serializer::deserialize(p.y, span);
    }

    static void serialize(const ForeignPoint& p, JSON& json){
        json = {p.x, p.y};
    }

    static void deserialize(ForeignPoint& p, const JSON& json){
        p.x = json[0];
        p.y = json[1];
    }
};

namespace {
    static_assert(!Reflectable<ForeignPoint>);
    static_assert(!Serializer::BinarySerializable<ForeignPoint>);
    static_assert(!Serializer::JsonSerializable<ForeignPoint>);
    static_assert(Serializer::CustomBinarySerializable<ForeignPoint>);
    static_assert(Serializer::CustomJsonSerializable<ForeignPoint>);
}

TEST(SerializerCustomSerializer, BinaryRoundTrip)
{
    ForeignPoint original{7, 8};

    Binary::ByteArray buffer;
    Serializer::serialize(original, buffer);

    Binary::ByteView view(buffer);
    ForeignPoint restored;
    Serializer::deserialize(restored, view);

    ASSERT_EQ(restored.x, 7);
    ASSERT_EQ(restored.y, 8);
}

TEST(SerializerCustomSerializer, JsonRoundTrip)
{
    ForeignPoint original{9, 10};

    JSON json;
    Serializer::serialize(original, json);

    ForeignPoint restored;
    Serializer::deserialize(restored, json);

    ASSERT_EQ(restored.x, 9);
    ASSERT_EQ(restored.y, 10);
}

TEST(SerializerJsonLike, DeserializeAcceptsAnRvalueJson)
{
    Path restored;
    Serializer::deserialize(restored, JSON{{"name", "trail"}, {"waypoints", {1, 2, 3}}});

    ASSERT_EQ(restored.name, "trail");
    ASSERT_EQ(restored.waypoints, (std::vector<int>{1, 2, 3}));
}

TEST(SerializerJsonLike, BinaryByteArrayNeedsNoCastToPickTheBinaryTier)
{
    // Binary::ByteArray (std::vector<std::byte>) happens to also be constructible into a JSON
    Point original{3, 4};

    Binary::ByteArray buffer;
    Serializer::serialize(original, buffer);

    Point restored;
    Serializer::deserialize(restored, buffer);

    ASSERT_EQ(restored.x, 3);
    ASSERT_EQ(restored.y, 4);
}