#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/asset/asset_manager.hpp"
#include "draft/rendering/animation.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace Draft;

namespace {
    // Array-form ("Array" export type) frames, no tags/slices/layers, exercises the minimal
    // shape as well as the array-vs-hash frame parsing branch.
    const char* THREE_FRAME_JSON = R"({
        "frames": [
            {"filename": "f0", "frame": {"x": 0, "y": 0, "w": 16, "h": 16}, "duration": 100},
            {"filename": "f1", "frame": {"x": 16, "y": 0, "w": 16, "h": 16}, "duration": 100},
            {"filename": "f2", "frame": {"x": 32, "y": 0, "w": 16, "h": 16}, "duration": 100}
        ],
        "meta": {"image": "three_frame.png", "size": {"w": 48, "h": 16}}
    })";

    const char* EMPTY_FRAMES_JSON = R"({
        "frames": [],
        "meta": {"image": "empty.png"}
    })";

    const char* ZERO_DURATION_JSON = R"({
        "frames": [
            {"filename": "f0", "frame": {"x": 0, "y": 0, "w": 16, "h": 16}, "duration": 0},
            {"filename": "f1", "frame": {"x": 16, "y": 0, "w": 16, "h": 16}, "duration": 0}
        ],
        "meta": {"image": "zero_duration.png"}
    })";

    // Hash-form frames named so lexicographic order ("sheet 10" before "sheet 2") diverges from
    // the animation's actual sequence.
    const char* HASH_ORDER_JSON = R"({
        "frames": {
            "sheet 0.aseprite": {"frame": {"x": 0, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 1.aseprite": {"frame": {"x": 16, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 2.aseprite": {"frame": {"x": 32, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 3.aseprite": {"frame": {"x": 48, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 4.aseprite": {"frame": {"x": 64, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 5.aseprite": {"frame": {"x": 80, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 6.aseprite": {"frame": {"x": 96, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 7.aseprite": {"frame": {"x": 112, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 8.aseprite": {"frame": {"x": 128, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 9.aseprite": {"frame": {"x": 144, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 10.aseprite": {"frame": {"x": 160, "y": 0, "w": 16, "h": 16}, "duration": 100},
            "sheet 11.aseprite": {"frame": {"x": 176, "y": 0, "w": 16, "h": 16}, "duration": 100}
        },
        "meta": {"image": "hash_order.png", "size": {"w": 192, "h": 16}}
    })";

    // Hash-form ("Hash" export type) frames plus frameTags/layers/slices.
    const char* FULL_JSON = R"({
        "frames": {
            "walk 0.aseprite": {"frame": {"x": 0, "y": 0, "w": 16, "h": 16}, "rotated": false, "trimmed": true, "spriteSourceSize": {"x": 1, "y": 2, "w": 16, "h": 16}, "sourceSize": {"w": 18, "h": 18}, "duration": 100, "data": "step"},
            "walk 1.aseprite": {"frame": {"x": 16, "y": 0, "w": 16, "h": 16}, "rotated": false, "trimmed": false, "spriteSourceSize": {"x": 0, "y": 0, "w": 16, "h": 16}, "sourceSize": {"w": 16, "h": 16}, "duration": 100},
            "walk 2.aseprite": {"frame": {"x": 32, "y": 0, "w": 16, "h": 16}, "rotated": false, "trimmed": false, "spriteSourceSize": {"x": 0, "y": 0, "w": 16, "h": 16}, "sourceSize": {"w": 16, "h": 16}, "duration": 100},
            "walk 3.aseprite": {"frame": {"x": 48, "y": 0, "w": 16, "h": 16}, "rotated": false, "trimmed": false, "spriteSourceSize": {"x": 0, "y": 0, "w": 16, "h": 16}, "sourceSize": {"w": 16, "h": 16}, "duration": 100}
        },
        "meta": {
            "app": "http://www.aseprite.org/",
            "version": "1.3.17.2-x64",
            "image": "walk.png",
            "format": "RGBA8888",
            "size": {"w": 64, "h": 16},
            "scale": "1",
            "frameTags": [
                {"name": "idle", "from": 0, "to": 0, "direction": "forward", "color": "#ff0000ff"},
                {"name": "walk", "from": 0, "to": 3, "direction": "forward", "color": "#00ff00ff", "data": "loops"},
                {"name": "walk_reverse", "from": 0, "to": 3, "direction": "reverse", "color": "#0000ffff"},
                {"name": "walk_pingpong", "from": 0, "to": 3, "direction": "pingpong", "color": "#ffff00ff"}
            ],
            "layers": [
                {"name": "Layer", "opacity": 255, "blendMode": "normal"}
            ],
            "slices": [
                {"name": "hitbox", "color": "#0000ffff", "data": "solid", "keys": [
                    {"frame": 0, "bounds": {"x": 1, "y": 2, "w": 10, "h": 11}, "center": {"x": 2, "y": 3, "w": 4, "h": 5}, "pivot": {"x": 6, "y": 7}}
                ]}
            ]
        }
    })";

    void write_file(const std::filesystem::path& path, const std::string& contents){
        std::ofstream out(path);
        out << contents;
    }
}

// Animation resolves and loads its own spritesheet texture via AssetManager, so the whole
// suite shares one hidden RenderWindow/GL context plus a scratch directory on disk (the
// default AssetManager only reads from disk/archive/embedded providers).
class AnimationTest : public ::testing::Test {
protected:
    static RenderWindow* window;
    std::filesystem::path dir = "test_animation_scratch";
    AssetManager assets;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "animation_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    void SetUp() override {
        std::filesystem::create_directories(dir);

        // Real image decoding needs real image bytes; tests only care that Animation resolves
        // and requests the right key, so swap in a loader that fabricates a texture instead.
        assets.register_loader<Texture>([](const FileHandle&, AssetManager&){
            return Texture(Image({64, 16}, {1, 1, 1, 1}));
        });
    }

    void TearDown() override {
        std::filesystem::remove_all(dir);
    }

    std::string path(const std::string& name) const {
        return (dir / name).string();
    }

    // Writes @p json to @p jsonName plus a placeholder file for whatever meta.image names, then
    // constructs the Animation from it.
    Animation load(const std::string& jsonName, const std::string& json, const std::string& imageName){
        write_file(path(jsonName), json);
        write_file(path(imageName), "");
        return Animation(assets.file_system().open(path(jsonName)), assets);
    }
};

RenderWindow* AnimationTest::window = nullptr;

TEST_F(AnimationTest, GetFrameReturnsTheFrameCoveringTheGivenTime)
{
    Animation anim = load("three_frame.json", THREE_FRAME_JSON, "three_frame.png");

    EXPECT_FLOAT_EQ(anim.get_frame(50.f).bounds.x, 0.f);
    EXPECT_FLOAT_EQ(anim.get_frame(150.f).bounds.x, 16.f);
    EXPECT_FLOAT_EQ(anim.get_frame(250.f).bounds.x, 32.f);
}

TEST_F(AnimationTest, GetFrameWrapsAroundPastTheTotalDuration)
{
    Animation anim = load("wrap.json", THREE_FRAME_JSON, "three_frame.png");

    // totalFrameTime is 300, 350 wraps to 50, landing back in the first frame.
    EXPECT_FLOAT_EQ(anim.get_frame(350.f).bounds.x, 0.f);
}

TEST_F(AnimationTest, ConstructingWithNoFramesThrowsFromGetFrameInsteadOfIndexingOutOfBounds)
{
    Animation anim = load("empty.json", EMPTY_FRAMES_JSON, "empty.png");

    EXPECT_THROW(anim.get_frame(0.f), std::runtime_error);
}

TEST_F(AnimationTest, AllZeroDurationFramesFallBackToTheLastFrameNotAnOutOfBoundsIndex)
{
    // totalFrameTime stays at 0 despite 2 real frames existing, every frameEndTimes[i] > frameTime
    // check is false, so the fallback path (frames.back()) is exercised instead of indexing OOB.
    Animation anim = load("zero_duration.json", ZERO_DURATION_JSON, "zero_duration.png");

    EXPECT_FLOAT_EQ(anim.get_frame(5.f).bounds.x, 16.f); // frames.back()'s x
}

TEST_F(AnimationTest, ParsesHashFormFramesWithNameAndFullPerFrameMetadata)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    ASSERT_EQ(anim.get_frames().size(), 4u);

    const AnimationFrame& first = anim.get_frames()[0];
    EXPECT_EQ(first.name, "walk 0.aseprite");
    EXPECT_TRUE(first.trimmed);
    EXPECT_FALSE(first.rotated);
    EXPECT_FLOAT_EQ(first.duration, 100.f);
    EXPECT_FLOAT_EQ(first.spriteSourceSize.x, 1.f);
    EXPECT_FLOAT_EQ(first.spriteSourceSize.y, 2.f);
    EXPECT_FLOAT_EQ(first.spriteSourceSize.width, 16.f);
    EXPECT_FLOAT_EQ(first.sourceSize.x, 18.f);
    EXPECT_FLOAT_EQ(first.sourceSize.y, 18.f);
    EXPECT_EQ(first.userData, "step");

    EXPECT_EQ(anim.get_frames()[1].userData, "");
    EXPECT_FLOAT_EQ(anim.get_total_frame_time(), 400.f);
}

TEST_F(AnimationTest, HashFormFramesKeepFileOrderNotLexicographicKeyOrder)
{
    // Regression test for json ordering
    Animation anim = load("hash_order.json", HASH_ORDER_JSON, "hash_order.png");

    ASSERT_EQ(anim.get_frames().size(), 12u);
    for(size_t i = 0; i < anim.get_frames().size(); i++){
        EXPECT_EQ(anim.get_frames()[i].name, "sheet " + std::to_string(i) + ".aseprite");
        EXPECT_FLOAT_EQ(anim.get_frames()[i].frame.x, static_cast<float>(i * 16));
    }
}

TEST_F(AnimationTest, ParsesMetaFields)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    EXPECT_EQ(anim.get_app(), "http://www.aseprite.org/");
    EXPECT_EQ(anim.get_version(), "1.3.17.2-x64");
    EXPECT_EQ(anim.get_image_filename(), "walk.png");
    EXPECT_EQ(anim.get_format(), "RGBA8888");
    EXPECT_EQ(anim.get_sheet_size().x, 64u);
    EXPECT_EQ(anim.get_sheet_size().y, 16u);
    EXPECT_FLOAT_EQ(anim.get_scale(), 1.f);
}

TEST_F(AnimationTest, ParsesFrameTagsWithNameRangeDirectionColorAndUserData)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    ASSERT_TRUE(anim.has_tag("walk"));
    const AnimationTag& walk = anim.get_tag("walk");
    EXPECT_EQ(walk.from, 0);
    EXPECT_EQ(walk.to, 3);
    EXPECT_EQ(walk.direction, AnimationDirection::Forward);
    EXPECT_EQ(walk.color.to_hex(true), "#00ff00ff");
    EXPECT_EQ(walk.userData, "loops");

    EXPECT_FALSE(anim.has_tag("does_not_exist"));
}

TEST_F(AnimationTest, GetFrameByTagLoopsWithinThatTagsOwnDuration)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    EXPECT_FLOAT_EQ(anim.get_frame("idle", 0.f).bounds.x, 0.f);
    EXPECT_FLOAT_EQ(anim.get_frame("idle", 250.f).bounds.x, 0.f); // idle is just frame 0, wraps forever

    EXPECT_FLOAT_EQ(anim.get_frame("walk", 50.f).bounds.x, 0.f);
    EXPECT_FLOAT_EQ(anim.get_frame("walk", 150.f).bounds.x, 16.f);
    EXPECT_FLOAT_EQ(anim.get_frame("walk", 350.f).bounds.x, 48.f);
    EXPECT_FLOAT_EQ(anim.get_frame("walk", 450.f).bounds.x, 0.f); // wraps past the tag's 400 total
}

TEST_F(AnimationTest, GetFrameByTagRespectsReverseDirection)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    EXPECT_FLOAT_EQ(anim.get_frame("walk_reverse", 50.f).bounds.x, 48.f);
    EXPECT_FLOAT_EQ(anim.get_frame("walk_reverse", 150.f).bounds.x, 32.f);
    EXPECT_FLOAT_EQ(anim.get_frame("walk_reverse", 350.f).bounds.x, 0.f);
}

TEST_F(AnimationTest, GetFrameByTagRespectsPingPongDirection)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    // Forward pass: 0, 1, 2, 3 then back down without repeating the endpoints: 2, 1
    EXPECT_FLOAT_EQ(anim.get_frame("walk_pingpong", 350.f).bounds.x, 48.f); // still climbing (frame 3)
    EXPECT_FLOAT_EQ(anim.get_frame("walk_pingpong", 450.f).bounds.x, 32.f); // bounced, back to frame 2
    EXPECT_FLOAT_EQ(anim.get_frame("walk_pingpong", 550.f).bounds.x, 16.f); // frame 1
    EXPECT_FLOAT_EQ(anim.get_frame("walk_pingpong", 650.f).bounds.x, 0.f); // wrapped (600 total), back to frame 0
}

TEST_F(AnimationTest, GetFrameThrowsForAnUnknownTag)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    EXPECT_THROW(anim.get_frame("does_not_exist", 0.f), std::runtime_error);
}

TEST_F(AnimationTest, ParsesSlicesWithBoundsCenterAndPivot)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    ASSERT_EQ(anim.get_slices().size(), 1u);
    const AnimationSlice& slice = anim.get_slices()[0];
    EXPECT_EQ(slice.name, "hitbox");
    EXPECT_EQ(slice.userData, "solid");
    EXPECT_EQ(slice.color.to_hex(true), "#0000ffff");

    ASSERT_EQ(slice.keys.size(), 1u);
    const AnimationSliceKey& key = slice.keys[0];
    EXPECT_EQ(key.frame, 0);
    EXPECT_EQ(key.bounds.x, 1);
    EXPECT_EQ(key.bounds.y, 2);
    EXPECT_EQ(key.bounds.width, 10);
    EXPECT_EQ(key.bounds.height, 11);

    ASSERT_TRUE(key.center.has_value());
    EXPECT_EQ(key.center->x, 2);
    EXPECT_EQ(key.center->width, 4);

    ASSERT_TRUE(key.pivot.has_value());
    EXPECT_EQ(key.pivot->x, 6);
    EXPECT_EQ(key.pivot->y, 7);
}

TEST_F(AnimationTest, ParsesLayersWithOpacityAndBlendMode)
{
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    ASSERT_EQ(anim.get_layers().size(), 1u);
    const AnimationLayer& layer = anim.get_layers()[0];
    EXPECT_EQ(layer.name, "Layer");
    EXPECT_FLOAT_EQ(layer.opacity, 255.f);
    EXPECT_EQ(layer.blendMode, "normal");
}

TEST_F(AnimationTest, ResolvesTextureRelativeToTheJsonsOwnDirectory)
{
    // meta.image is a bare filename ("walk.png"); Animation must resolve it against the
    // JSON's own directory (dir/walk.png), not the current working directory.
    Animation anim = load("full.json", FULL_JSON, "walk.png");

    ASSERT_TRUE(anim.get_texture().is_valid());
    EXPECT_EQ(anim.get_texture()->get_properties().size.x, 64u);
}
