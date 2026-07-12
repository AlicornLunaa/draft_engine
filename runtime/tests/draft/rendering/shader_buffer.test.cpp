#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/shader_buffer.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    struct TestBlock {
        float a;
        float b;
        float c;
        float d;
    };
}

class ShaderBufferTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "shader_buffer_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* ShaderBufferTest::window = nullptr;

TEST_F(ShaderBufferTest, ConstructionUploadsInitialData)
{
    TestBlock initial{1.f, 2.f, 3.f, 4.f};
    ShaderBuffer<TestBlock> buffer(0, &initial);
    buffer.bind();

    TestBlock readback{};
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(TestBlock), &readback);
    EXPECT_FLOAT_EQ(readback.a, 1.f);
    EXPECT_FLOAT_EQ(readback.d, 4.f);

    buffer.unbind();
}

TEST_F(ShaderBufferTest, SetReuploadsTheWholeBlock)
{
    ShaderBuffer<TestBlock> buffer;
    buffer.set({10.f, 20.f, 30.f, 40.f});
    buffer.bind();

    TestBlock readback{};
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(TestBlock), &readback);
    EXPECT_FLOAT_EQ(readback.a, 10.f);
    EXPECT_FLOAT_EQ(readback.c, 30.f);

    buffer.unbind();
}

TEST_F(ShaderBufferTest, DestructorActuallyDeletesTheGLBuffer)
{
    unsigned int rawHandle = 0;

    {
        ShaderBuffer<TestBlock> buffer;
        buffer.bind();
        GLint boundId = 0;
        glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &boundId);
        rawHandle = static_cast<unsigned int>(boundId);
        ASSERT_NE(rawHandle, 0u);
        buffer.unbind();
    }

    EXPECT_FALSE(glIsBuffer(rawHandle));
}

TEST_F(ShaderBufferTest, MoveConstructionTransfersOwnership)
{
    unsigned int rawHandle = 0;

    ShaderBuffer<TestBlock> original;
    original.bind();
    GLint boundId = 0;
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &boundId);
    rawHandle = static_cast<unsigned int>(boundId);
    original.unbind();

    ShaderBuffer<TestBlock> moved(std::move(original));
    EXPECT_TRUE(glIsBuffer(rawHandle));

    moved.bind();
    GLint stillBoundId = 0;
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &stillBoundId);
    EXPECT_EQ(static_cast<unsigned int>(stillBoundId), rawHandle);
    moved.unbind();
}
