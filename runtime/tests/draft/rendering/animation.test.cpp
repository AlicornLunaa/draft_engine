#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/animation.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/virtual_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include <stdexcept>

using namespace Draft;

namespace {
    const char* THREE_FRAME_JSON = R"({
        "frames": [
            {"frame": {"x": 0, "y": 0, "w": 16, "h": 16}, "duration": 100},
            {"frame": {"x": 16, "y": 0, "w": 16, "h": 16}, "duration": 100},
            {"frame": {"x": 32, "y": 0, "w": 16, "h": 16}, "duration": 100}
        ],
        "meta": {"image": "spritesheet.png"}
    })";

    const char* EMPTY_FRAMES_JSON = R"({
        "frames": [],
        "meta": {"image": "spritesheet.png"}
    })";

    const char* ZERO_DURATION_JSON = R"({
        "frames": [
            {"frame": {"x": 0, "y": 0, "w": 16, "h": 16}, "duration": 0},
            {"frame": {"x": 16, "y": 0, "w": 16, "h": 16}, "duration": 0}
        ],
        "meta": {"image": "spritesheet.png"}
    })";
}

// Animation itself has no direct GL calls, but building a real Resource<Texture> to hand it
// does, so the whole suite shares one hidden RenderWindow/GL context.
class AnimationTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "animation_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    static Resource<Texture> make_texture(){
        auto texture = std::make_shared<Texture>(Image({48, 16}, {1, 1, 1, 1}));
        return Resource<Texture>(std::make_shared<AssetSlot<Texture>>(std::move(texture)));
    }
};

RenderWindow* AnimationTest::window = nullptr;

TEST_F(AnimationTest, GetFrameReturnsTheFrameCoveringTheGivenTime)
{
    VirtualFileSystem fs;
    fs.write_string("anim_three_frame.json", THREE_FRAME_JSON);
    Animation anim(make_texture(), fs.open("anim_three_frame.json"));

    EXPECT_FLOAT_EQ(anim.get_frame(50.f).bounds.x, 0.f);
    EXPECT_FLOAT_EQ(anim.get_frame(150.f).bounds.x, 16.f);
    EXPECT_FLOAT_EQ(anim.get_frame(250.f).bounds.x, 32.f);
}

TEST_F(AnimationTest, GetFrameWrapsAroundPastTheTotalDuration)
{
    VirtualFileSystem fs;
    fs.write_string("anim_wrap.json", THREE_FRAME_JSON);
    Animation anim(make_texture(), fs.open("anim_wrap.json"));

    // totalFrameTime is 300 - 350 wraps to 50, landing back in the first frame.
    EXPECT_FLOAT_EQ(anim.get_frame(350.f).bounds.x, 0.f);
}

TEST_F(AnimationTest, ConstructingWithNoFramesThrowsFromGetFrameInsteadOfIndexingOutOfBounds)
{
    VirtualFileSystem fs;
    fs.write_string("anim_empty.json", EMPTY_FRAMES_JSON);
    Animation anim(make_texture(), fs.open("anim_empty.json"));

    EXPECT_THROW(anim.get_frame(0.f), std::runtime_error);
}

TEST_F(AnimationTest, AllZeroDurationFramesFallBackToTheLastFrameNotAnOutOfBoundsIndex)
{
    // totalFrameTime stays at 0 despite 2 real frames existing - every frameTimes[i] > frameTime
    // check is false, so the old code fell through to `frames[(int)(frameTime / 100) % 20]`,
    // which is out of bounds for any frames.size() < 20 (here, 2). The fix falls back to
    // frames.back() instead.
    VirtualFileSystem fs;
    fs.write_string("anim_zero_duration.json", ZERO_DURATION_JSON);
    Animation anim(make_texture(), fs.open("anim_zero_duration.json"));

    EXPECT_FLOAT_EQ(anim.get_frame(5.f).bounds.x, 16.f); // frames.back()'s x
}
