#include <gtest/gtest.h>
#include "draft/util/json.hpp"
#include "draft/util/files/disk_file_provider.hpp"
#include <string>
#include <utility>

using namespace Draft;

TEST(JSON, ParsesFromString)
{
    JSON j = JSON::parse(R"({"a":1,"b":"two"})");
    ASSERT_EQ(j["a"], 1);
    ASSERT_EQ(j["b"], "two");
}

TEST(JSON, ConstructsFromFileHandle)
{
    DiskFileProvider provider;
    provider.write_string("test_json_file.json", R"({"value":42})");

    JSON j(provider.open("test_json_file.json"));
    ASSERT_EQ(j["value"], 42);

    provider.remove("test_json_file.json");
}

TEST(JSON, ConstructsFromNlohmannJsonCopyAndMove)
{
    nlohmann::json source = { {"key", "value"} };

    JSON copied(source);
    ASSERT_EQ(copied["key"], "value");

    JSON moved(std::move(source));
    ASSERT_EQ(moved["key"], "value");
}

namespace {
    // A minimal concrete JSONInterface for exercising encode_json()/decode_json().
    struct Point : Draft::JSONInterface {
        int x = 0;
        int y = 0;

        std::string encode() const override {
            JSON j;
            j["x"] = x;
            j["y"] = y;
            return j.dump();
        }

        void decode(const std::string& data) override {
            JSON j = JSON::parse(data);
            x = j["x"];
            y = j["y"];
        }
    };
}

TEST(JSONInterface, EncodeRoundTripsThroughDecode)
{
    Point original;
    original.x = 3;
    original.y = 7;

    std::string encoded = encode_json(original);

    Point decoded;
    decode_json(decoded, encoded);

    ASSERT_EQ(decoded.x, 3);
    ASSERT_EQ(decoded.y, 7);
}
