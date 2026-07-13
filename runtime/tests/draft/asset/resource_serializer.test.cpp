#include <gtest/gtest.h>
#include "draft/util/serialization/resource_serializer.hpp"

#include <filesystem>
#include <fstream>

using namespace Draft;

static_assert(Serializer::Serializable<Resource<int>>);
static_assert(Serializer::BinarySerializableWithAdvance<Resource<int>>);

namespace {
    // A trivial asset type, just the file's contents - mirrors asset_manager.test.cpp's TextAsset.
    struct TextAsset {
        std::string contents;
    };

    void write_file(const std::filesystem::path& path, const std::string& contents){
        std::ofstream out(path);
        out << contents;
    }

    struct ResourceSerializerTest : ::testing::Test {
        std::filesystem::path dir = "test_resource_serializer_scratch";
        AssetManager manager;
        SceneSerializationContext ctx{&manager, {}, {}};

        void SetUp() override {
            std::filesystem::create_directories(dir);
            manager.register_loader<TextAsset>([](const FileHandle& handle, AssetManager&){
                return TextAsset{handle.read_string()};
            });
        }

        void TearDown() override {
            std::filesystem::remove_all(dir);
        }

        std::string path(const std::string& name) const { return (dir / name).string(); }
    };
}

TEST_F(ResourceSerializerTest, JsonRoundTripResolvesToTheSameKey)
{
    write_file(path("a.txt"), "hello");
    Resource<TextAsset> original = manager.get<TextAsset>(path("a.txt"));

    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

    JSON json;
    Serializer::serialize(original, json);
    ASSERT_EQ(json.get<std::string>(), path("a.txt"));

    Resource<TextAsset> restored;
    Serializer::deserialize(restored, json);

    ASSERT_TRUE(restored.is_valid());
    ASSERT_EQ(restored->contents, "hello");
    ASSERT_EQ(restored.slot_id(), original.slot_id());
}

TEST_F(ResourceSerializerTest, BinaryRoundTripResolvesToTheSameKey)
{
    write_file(path("a.txt"), "hello");
    Resource<TextAsset> original = manager.get<TextAsset>(path("a.txt"));

    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

    Binary::ByteArray buffer;
    Serializer::serialize(original, buffer);

    Binary::ByteView view(buffer);
    Resource<TextAsset> restored;
    Serializer::deserialize(restored, view);

    ASSERT_TRUE(restored.is_valid());
    ASSERT_EQ(restored->contents, "hello");
    ASSERT_EQ(restored.slot_id(), original.slot_id());
}

TEST_F(ResourceSerializerTest, InvalidResourceRoundTripsToInvalid)
{
    Resource<TextAsset> empty;

    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

    JSON json;
    Serializer::serialize(empty, json);
    ASSERT_EQ(json.get<std::string>(), "");

    // Starts pointed at a loaded asset, deserializing an empty key should overwrite it
    // back to invalid rather than leaving the previous value alone.
    write_file(path("a.txt"), "hello");
    Resource<TextAsset> restored = manager.get<TextAsset>(path("a.txt"));
    ASSERT_TRUE(restored.is_valid());

    Serializer::deserialize(restored, json);
    ASSERT_FALSE(restored.is_valid());
}

TEST_F(ResourceSerializerTest, BinaryDeserializeAndAdvanceLeavesTrailingDataIntact)
{
    write_file(path("a.txt"), "hello");
    write_file(path("b.txt"), "world");
    Resource<TextAsset> first = manager.get<TextAsset>(path("a.txt"));
    Resource<TextAsset> second = manager.get<TextAsset>(path("b.txt"));

    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

    Binary::ByteArray buffer;
    Serializer::serialize(first, buffer);
    Serializer::serialize(second, buffer);

    Binary::ByteView view(buffer);
    Resource<TextAsset> restoredFirst, restoredSecond;
    Serializer::deserialize_and_advance(restoredFirst, view);
    Serializer::deserialize_and_advance(restoredSecond, view);

    ASSERT_TRUE(view.empty());
    ASSERT_EQ(restoredFirst->contents, "hello");
    ASSERT_EQ(restoredSecond->contents, "world");
}
