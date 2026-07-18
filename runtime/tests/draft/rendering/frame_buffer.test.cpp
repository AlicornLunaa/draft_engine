#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

// Every Framebuffer operation (even the constructor) issues real GL calls, so the whole suite
// shares one hidden RenderWindow/GL context instead of creating one per test.
class FramebufferTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "frame_buffer_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* FramebufferTest::window = nullptr;

TEST_F(FramebufferTest, ConstructionProducesACompleteFramebuffer)
{
    Framebuffer fb({{8, 8}});
    fb.begin();
    EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
    fb.end();
}

TEST_F(FramebufferTest, AttachmentTexturesHaveRealHandlesAtTheRightSize)
{
    Framebuffer fb({{16, 12}});

    EXPECT_NE(fb.get_texture().get_texture_handle(), 0u);
    EXPECT_EQ(fb.get_texture().get_properties().size.x, 16u);
    EXPECT_EQ(fb.get_texture().get_properties().size.y, 12u);

    EXPECT_NE(fb.get_depth_texture().get_texture_handle(), 0u);
    EXPECT_EQ(fb.get_depth_texture().get_properties().size.x, 16u);
    EXPECT_EQ(fb.get_depth_texture().get_properties().size.y, 12u);
}

TEST_F(FramebufferTest, BeginEndNestsCorrectlyAgainstAPreviouslyBoundFramebuffer)
{
    Framebuffer outer({{8, 8}});
    Framebuffer inner({{4, 4}});

    outer.begin();
    GLint outerFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &outerFbo);

    inner.begin();
    GLint innerFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &innerFbo);
    EXPECT_NE(innerFbo, outerFbo);
    inner.end();

    // Ending the inner pass must restore the outer one, not the default framebuffer (0)
    GLint restoredFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &restoredFbo);
    EXPECT_EQ(restoredFbo, outerFbo);

    outer.end();
}

TEST_F(FramebufferTest, ResizeRegeneratesAttachmentsAtTheNewSize)
{
    Framebuffer fb({{8, 8}});
    fb.resize({32, 24});

    EXPECT_EQ(fb.get_size().x, 32u);
    EXPECT_EQ(fb.get_size().y, 24u);
    EXPECT_EQ(fb.get_texture().get_properties().size.x, 32u);
    EXPECT_EQ(fb.get_texture().get_properties().size.y, 24u);
}

TEST_F(FramebufferTest, ClearActuallyClearsTheColorAttachment)
{
    Framebuffer fb({{4, 4}});
    fb.begin({0.25f, 0.5f, 0.75f, 1.f});

    unsigned char pixel[4] = {};
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    fb.end();

    EXPECT_NEAR(pixel[0], 0.25f * 255, 2);
    EXPECT_NEAR(pixel[1], 0.5f * 255, 2);
    EXPECT_NEAR(pixel[2], 0.75f * 255, 2);
}
