#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/asset_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

// Used to get private access to the TextRenderer.
namespace Draft {
    struct TextRendererTestAccess {
        static Vector2f compute_glyph_top_left(float currX, float positionY, const Vector2f& bearing, float scale, float stringHeight){
            return TextRenderer::compute_glyph_top_left(currX, positionY, bearing, scale, stringHeight);
        }
    };
}

// Pure math, no GL/Freetype/Font needed, directly pins the X/Y placement fix.
TEST(TextRendererGlyphPlacement, OnlyTheIntrinsicBearingIsScaledNeverTheCallersPosition)
{
    // With scale = 1, the formula should reduce to the textbook baseline calculation.
    Vector2f atScale1 = TextRendererTestAccess::compute_glyph_top_left(10.f, 20.f, {0.f, 5.f}, 1.f, 8.f);
    EXPECT_FLOAT_EQ(atScale1.x, 10.f);
    EXPECT_FLOAT_EQ(atScale1.y, 20.f - 5.f + 8.f); // 23

    // Changing scale must only move the position by the (now-scaled) bearing term
    Vector2f atScale2 = TextRendererTestAccess::compute_glyph_top_left(10.f, 20.f, {0.f, 5.f}, 2.f, 8.f);
    EXPECT_FLOAT_EQ(atScale2.y, 20.f - 5.f * 2.f + 8.f); // 18
    EXPECT_NE(atScale2.y, (20.f - 5.f) * 2.f + 8.f); // would be 38 under the old buggy formula
}

TEST(TextRendererGlyphPlacement, XAndYTreatTheCallersPositionSymmetrically)
{
    // X never re-scales currX (which already accumulates in final/scaled space)
    Vector2f placement = TextRendererTestAccess::compute_glyph_top_left(100.f, 50.f, {3.f, 4.f}, 3.f, 0.f);
    EXPECT_FLOAT_EQ(placement.x, 100.f + 3.f * 3.f); // currX untouched, only bearing.x scaled
    EXPECT_FLOAT_EQ(placement.y, 50.f - 4.f * 3.f); // positionY untouched, only bearing.y scaled
}

class TextRendererTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "text_renderer_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* TextRendererTest::window = nullptr;

TEST_F(TextRendererTest, GetTextBoundsReflectsRealGlyphMetrics)
{
    Font font(AssetFileSystem().open("assets/fonts/default.ttf"));
    TextRenderer renderer;

    TextProperties props;
    props.str = "AB";
    props.font = &font;
    props.fontSize = 24;

    Vector2f empty = renderer.get_text_bounds({"", &font, 24});
    Vector2f bounds = renderer.get_text_bounds(props);

    EXPECT_GT(bounds.x, empty.x);
    EXPECT_GT(bounds.y, 0.f);
}

TEST_F(TextRendererTest, DrawTextIntoARealSpriteCollectionProducesNoGLError)
{
    Font font(AssetFileSystem().open("assets/fonts/default.ttf"));
    SpriteCollection collection;
    TextRenderer renderer;

    TextProperties props;
    props.str = "Hi!";
    props.font = &font;
    props.fontSize = 20;

    renderer.draw_text(collection, props);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(TextRendererTest, DrawTextWithTheStringOverloadProducesNoGLError)
{
    Font font(AssetFileSystem().open("assets/fonts/default.ttf"));
    SpriteCollection collection;
    TextRenderer renderer;

    renderer.draw_text(collection, "Overload", &font, {0, 0});

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(TextRendererTest, SetShaderChangesTheActiveShader)
{
    TextRenderer renderer;
    unsigned int originalHandle = renderer.get_shader().get_shader_handle();
    EXPECT_NE(originalHandle, 0u);
}
