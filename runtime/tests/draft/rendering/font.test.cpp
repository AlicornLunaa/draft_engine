#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/font.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/asset_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

// Every Font operation (even construction) issues real GL calls (baking glyphs uploads real
// Textures), so the whole suite shares one hidden RenderWindow/GL context instead of creating
// one per test.
class FontTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "font_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* FontTest::window = nullptr;

TEST_F(FontTest, ConstructionFromEmbeddedFontBakesRealGlyphs)
{
    Font font(AssetFileSystem().open("assets/fonts/default.ttf"));

    const Font::Glyph& glyph = font.get_glyph('A');
    EXPECT_NE(glyph.region.texture->get_texture_handle(), 0u);
    EXPECT_GT(glyph.size.x, 0.f);
}

TEST_F(FontTest, DifferentFontSizesGetDistinctAtlasTextures)
{
    Font font(AssetFileSystem().open("assets/fonts/default.ttf"));

    font.set_font_size(24);
    const Font::Glyph& small = font.get_glyph('A');

    font.set_font_size(48);
    const Font::Glyph& large = font.get_glyph('A');

    EXPECT_NE(small.region.texture->get_texture_handle(), large.region.texture->get_texture_handle());
    // A larger font size should rasterize a visibly bigger glyph bitmap.
    EXPECT_GT(large.size.x, small.size.x);
}

TEST_F(FontTest, DefaultAtlasPropertiesSetBothWrapAxesToClampToEdge)
{
    // Regression test for the same duplicate-TEXTURE_WRAP_S-key shape already fixed in
    // Texture's/Framebuffer's own defaults.
    Font font(AssetFileSystem().open("assets/fonts/default.ttf"));
    const Font::Glyph& glyph = font.get_glyph('A');

    glyph.region.texture->bind();
    GLint wrapS = 0, wrapT = 0;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrapS);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrapT);
    glyph.region.texture->unbind();

    EXPECT_EQ(wrapS, GL_CLAMP_TO_EDGE);
    EXPECT_EQ(wrapT, GL_CLAMP_TO_EDGE);
}

TEST_F(FontTest, BakingEnoughGlyphsToOverflowAPageProducesValidRegionsThroughout)
{
    // Regression test for the bake_glyph() reference-invalidation bug
    Font font(AssetFileSystem().open("assets/fonts/default.ttf"));
    font.set_font_size(1500);

    bool sawMoreThanOneTexture = false;
    unsigned int firstHandle = 0;

    for(char ch = 32; ch < 127; ch++){
        const Font::Glyph& glyph = font.get_glyph(ch);
        unsigned int handle = glyph.region.texture->get_texture_handle();
        ASSERT_NE(handle, 0u);

        if(firstHandle == 0){
            firstHandle = handle;
        } else if(handle != firstHandle){
            sawMoreThanOneTexture = true;
        }

        const Vector2u& textureSize = glyph.region.texture->get_properties().size;
        EXPECT_LE(glyph.region.bounds.x + glyph.region.bounds.width, textureSize.x)
            << "glyph '" << ch << "' bounds exceed its texture's real width";
        EXPECT_LE(glyph.region.bounds.y + glyph.region.bounds.height, textureSize.y)
            << "glyph '" << ch << "' bounds exceed its texture's real height";
    }

    EXPECT_TRUE(sawMoreThanOneTexture) << "test didn't actually force a page rollover - font size may need to be larger";
}

TEST_F(FontTest, ConstructingFromGarbageBytesThrowsInsteadOfExitingTheProcess)
{
    std::vector<std::byte> garbage(64, std::byte{0xFF});
    EXPECT_THROW(Font font(garbage), std::runtime_error);
}
