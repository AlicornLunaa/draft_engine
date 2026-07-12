#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/vertex_array.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

// Every VertexArray operation (even the constructor) issues real GL calls, so the whole suite
// shares one hidden RenderWindow/GL context instead of creating one per test.
class VertexArrayTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "vertex_array_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* VertexArrayTest::window = nullptr;

TEST_F(VertexArrayTest, CreateAllocatesRealVaoAndVboHandles)
{
    VertexArray vao;
    vao.create({StaticBuffer({{0, GL_FLOAT, 3, sizeof(float) * 3, 0}})});

    vao.bind();
    GLint boundVao = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundVao);
    EXPECT_NE(boundVao, 0);
    vao.unbind();

    GLint unboundVao = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &unboundVao);
    EXPECT_EQ(unboundVao, 0);
}

TEST_F(VertexArrayTest, SetDataOnStaticBufferUploadsRealData)
{
    VertexArray vao;
    vao.create({StaticBuffer({{0, GL_FLOAT, 3, sizeof(float) * 3, 0}})});

    std::vector<float> data{1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    vao.set_data(0, data);

    vao.bind();
    float readback[6] = {};
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(readback), readback);
    EXPECT_FLOAT_EQ(readback[0], 1.f);
    EXPECT_FLOAT_EQ(readback[5], 6.f);
    vao.unbind();
}

TEST_F(VertexArrayTest, SetDataOnDynamicBufferUploadsViaSubData)
{
    VertexArray vao;
    vao.create({DynamicBuffer(4 * sizeof(float), {{0, GL_FLOAT, 1, sizeof(float), 0}})});

    std::vector<float> data{10.f, 20.f, 30.f, 40.f};
    vao.set_data(0, data);

    vao.bind();
    float readback[4] = {};
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(readback), readback);
    EXPECT_FLOAT_EQ(readback[0], 10.f);
    EXPECT_FLOAT_EQ(readback[3], 40.f);
    vao.unbind();
}

TEST_F(VertexArrayTest, DynamicBufferUploadIsClampedToItsMaxBytes)
{
    // set_data must not overrun a DynamicBuffer's preallocated store even if given more data
    // than it was sized for. buffer_sub_data clamps to maxBytes.
    VertexArray vao;
    vao.create({DynamicBuffer(2 * sizeof(float), {{0, GL_FLOAT, 1, sizeof(float), 0}})});

    std::vector<float> tooMuch{1.f, 2.f, 3.f, 4.f};
    vao.set_data(0, tooMuch); // Should not crash/overrun

    vao.bind();
    float readback[2] = {};
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(readback), readback);
    EXPECT_FLOAT_EQ(readback[0], 1.f);
    EXPECT_FLOAT_EQ(readback[1], 2.f);
    vao.unbind();
}

TEST_F(VertexArrayTest, AttributeLayoutIsReadableViaGetVertexAttrib)
{
    VertexArray vao;
    vao.create({StaticBuffer({{0, GL_FLOAT, 3, sizeof(float) * 3, 0}})});
    vao.bind();

    GLint enabled = 0, size = 0, type = 0;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);

    EXPECT_EQ(enabled, GL_TRUE);
    EXPECT_EQ(size, 3);
    EXPECT_EQ(type, GL_FLOAT);

    vao.unbind();
}

TEST_F(VertexArrayTest, DestructorActuallyDeletesTheGLObjects)
{
    unsigned int rawVao = 0, rawVbo = 0;

    {
        VertexArray vao;
        vao.create({StaticBuffer({{0, GL_FLOAT, 3, sizeof(float) * 3, 0}})});
        vao.bind();

        GLint boundVao = 0, boundVbo = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundVao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundVbo);
        rawVao = static_cast<unsigned int>(boundVao);

        // ARRAY_BUFFER isn't left bound after create(), so read the vbo handle from the
        // attribute's binding point instead.
        GLint attribBinding = 0;
        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &attribBinding);
        rawVbo = static_cast<unsigned int>(attribBinding);

        vao.unbind();
    }

    EXPECT_FALSE(glIsVertexArray(rawVao));
    EXPECT_FALSE(glIsBuffer(rawVbo));
}
