#include <gtest/gtest.h>
#include "draft/util/serializer.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
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

        static void deserialize(Label& l, JSON& json) {
            l.name = json["name"];
            l.value = json["value"];
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
