#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/clip.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

class ClipTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "clip_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* ClipTest::window = nullptr;

TEST_F(ClipTest, BeginEnablesScissorTestAndSetsTheBox)
{
    Clip clip;
    clip.box = {1, 2, 3, 4};
    clip.begin();

    EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));

    GLint box[4] = {};
    glGetIntegerv(GL_SCISSOR_BOX, box);
    EXPECT_EQ(box[0], 1);
    EXPECT_EQ(box[1], 2);
    EXPECT_EQ(box[2], 3);
    EXPECT_EQ(box[3], 4);

    clip.end();
    EXPECT_FALSE(glIsEnabled(GL_SCISSOR_TEST));
}

TEST_F(ClipTest, NestedClipsRestoreTheParentBoxOnEnd)
{
    Clip outer;
    outer.box = {0, 0, 50, 50};
    outer.begin();

    Clip inner;
    inner.box = {5, 5, 10, 10};
    inner.begin();

    GLint innerBox[4] = {};
    glGetIntegerv(GL_SCISSOR_BOX, innerBox);
    EXPECT_EQ(innerBox[2], 10);

    inner.end();

    // Ending the inner clip should restore the outer clip's box, not disable scissoring, the
    // outer clip is still active.
    EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));
    GLint restoredBox[4] = {};
    glGetIntegerv(GL_SCISSOR_BOX, restoredBox);
    EXPECT_EQ(restoredBox[2], 50);

    outer.end();
    EXPECT_FALSE(glIsEnabled(GL_SCISSOR_TEST));
}
