#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/texture.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/asset/resource.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

// Every Texture operation (even the constructor) issues real GL calls, so the whole suite shares
// one hidden RenderWindow/GL context instead of creating one per test.
class TextureTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        // glfwWindowHint requires GLFW to already be initialized, but a prior fixture's
        // TearDownTestSuite may have just called glfwTerminate() (Window::~Window() terminates
        // once the last live window is gone) - glfwInit() is idempotent, so calling it here
        // guarantees the hint below actually takes effect regardless of test suite ordering.
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "texture_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* TextureTest::window = nullptr;

TEST_F(TextureTest, DefaultConstructionAllocatesARealGLHandle)
{
    Texture tex;
    EXPECT_NE(tex.get_texture_handle(), 0u);
}

TEST_F(TextureTest, BindIsBoundUnbindTrackUnitState)
{
    Texture tex;

    tex.bind(4);
    EXPECT_TRUE(tex.is_bound(4));

    tex.unbind();
    EXPECT_FALSE(tex.is_bound(4));
}

TEST_F(TextureTest, ConstructingFromImageUploadsRealDimensions)
{
    Image img({4, 8}, {1.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA);
    Texture tex(img);

    tex.bind(5);

    GLint width = 0, height = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    EXPECT_EQ(width, 4);
    EXPECT_EQ(height, 8);

    EXPECT_EQ(tex.get_properties().size.x, 4u);
    EXPECT_EQ(tex.get_properties().size.y, 8u);
    EXPECT_TRUE(tex.is_loaded());

    tex.unbind();
}

TEST_F(TextureTest, DefaultParametersSetBothWrapAxesToRepeat)
{
    Texture tex;
    tex.bind(6);

    GLint wrapS = 0, wrapT = 0;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrapS);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrapT);
    EXPECT_EQ(wrapS, GL_REPEAT);
    EXPECT_EQ(wrapT, GL_REPEAT);

    tex.unbind();
}

TEST_F(TextureTest, SetImageUploadsSubRegionBytes)
{
    Texture tex(Image({4, 4}, {0.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA));

    Image patch({2, 2}, {1.f, 1.f, 1.f, 1.f}, ColorFormat::RGBA);
    tex.set_image(patch, {0, 0, 2, 2});

    tex.bind(7);

    std::vector<unsigned char> pixels(4 * 4 * 4, 0);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // The patched 2x2 corner should now be white
    EXPECT_EQ(pixels[0], 255);
    EXPECT_EQ(pixels[1], 255);
    EXPECT_EQ(pixels[2], 255);
    EXPECT_EQ(pixels[3], 255);

    tex.unbind();
}

TEST_F(TextureTest, TextureRegionGetUvComputesNormalizedCoordinates)
{
    auto slot = std::make_shared<AssetSlot<Texture>>(
        std::make_shared<Texture>(Image({10, 20}, {0.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA)));

    TextureRegion region{Resource<Texture>(slot), FloatRect{2, 4, 5, 10}};
    auto [uvMin, uvMax] = region.get_uv();

    EXPECT_FLOAT_EQ(uvMin.x, 2.f / 10.f);
    EXPECT_FLOAT_EQ(uvMin.y, 4.f / 20.f);
    EXPECT_FLOAT_EQ(uvMax.x, 7.f / 10.f);
    EXPECT_FLOAT_EQ(uvMax.y, 14.f / 20.f);
}
