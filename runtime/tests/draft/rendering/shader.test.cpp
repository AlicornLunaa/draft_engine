#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/shader.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/virtual_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    const char* VALID_VERTEX_SRC =
        "#version 450 core\n"
        "void main(){\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    // References every uniform under test so the compiler can't optimize any of them away.
    const char* VALID_FRAGMENT_SRC =
        "#version 450 core\n"
        "out vec4 outColor;\n"
        "uniform float testFloat;\n"
        "uniform int testInt;\n"
        "uniform vec3 testVec3;\n"
        "uniform mat4 testMat4;\n"
        "uniform uvec4 testUVec4;\n"
        "void main(){\n"
        "    outColor = vec4(testFloat) + vec4(float(testInt)) + vec4(testVec3, 0.0) + testMat4[0] + vec4(testUVec4);\n"
        "}\n";

    const char* BROKEN_FRAGMENT_SRC =
        "#version 450 core\n"
        "out vec4 outColor;\n"
        "void main(){\n"
        "    this is not valid glsl;\n"
        "}\n";
}

class ShaderTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "shader_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* ShaderTest::window = nullptr;

TEST_F(ShaderTest, ConstructionFromValidSourceBindsAndUnbindsWithoutThrowing)
{
    VirtualFileSystem fs;
    fs.write_string("test_shader_vertex.glsl", VALID_VERTEX_SRC);
    fs.write_string("test_shader_fragment.glsl", VALID_FRAGMENT_SRC);

    Shader shader(fs.open("test_shader_vertex.glsl"), fs.open("test_shader_fragment.glsl"));
    EXPECT_NE(shader.get_shader_handle(), 0u);
    EXPECT_NO_THROW(shader.bind());
    EXPECT_NO_THROW(shader.unbind());

    fs.remove("test_shader_vertex.glsl");
    fs.remove("test_shader_fragment.glsl");
}

TEST_F(ShaderTest, HasUniformDistinguishesRealFromMissingNames)
{
    VirtualFileSystem fs;
    fs.write_string("test_shader_vertex2.glsl", VALID_VERTEX_SRC);
    fs.write_string("test_shader_fragment2.glsl", VALID_FRAGMENT_SRC);

    Shader shader(fs.open("test_shader_vertex2.glsl"), fs.open("test_shader_fragment2.glsl"));
    EXPECT_TRUE(shader.has_uniform("testFloat"));
    EXPECT_FALSE(shader.has_uniform("thisUniformDoesNotExist"));
    EXPECT_GE(shader.get_location("testFloat"), 0);

    fs.remove("test_shader_vertex2.glsl");
    fs.remove("test_shader_fragment2.glsl");
}

TEST_F(ShaderTest, SetUniformRoundTripsFloatIntVec3AndMat4)
{
    VirtualFileSystem fs;
    fs.write_string("test_shader_vertex3.glsl", VALID_VERTEX_SRC);
    fs.write_string("test_shader_fragment3.glsl", VALID_FRAGMENT_SRC);

    Shader shader(fs.open("test_shader_vertex3.glsl"), fs.open("test_shader_fragment3.glsl"));
    shader.bind();
    unsigned int program = shader.get_shader_handle();

    shader.set_uniform("testFloat", 3.5f);
    float readFloat = 0.f;
    glGetUniformfv(program, shader.get_location("testFloat"), &readFloat);
    EXPECT_FLOAT_EQ(readFloat, 3.5f);

    shader.set_uniform("testInt", 7);
    int readInt = 0;
    glGetUniformiv(program, shader.get_location("testInt"), &readInt);
    EXPECT_EQ(readInt, 7);

    shader.set_uniform("testVec3", Vector3f{1.f, 2.f, 3.f});
    float readVec3[3] = {};
    glGetUniformfv(program, shader.get_location("testVec3"), readVec3);
    EXPECT_FLOAT_EQ(readVec3[0], 1.f);
    EXPECT_FLOAT_EQ(readVec3[1], 2.f);
    EXPECT_FLOAT_EQ(readVec3[2], 3.f);

    Matrix4 mat(1.f);
    mat[3][0] = 42.f; // translation.x, distinctive value to check placement
    shader.set_uniform("testMat4", mat);
    float readMat[16] = {};
    glGetUniformfv(program, shader.get_location("testMat4"), readMat);
    EXPECT_FLOAT_EQ(readMat[12], 42.f); // column 3, row 0 in column-major layout

    fs.remove("test_shader_vertex3.glsl");
    fs.remove("test_shader_fragment3.glsl");
}

TEST_F(ShaderTest, SetUniformVector4uRoundTripsAllFourComponents)
{
    // Regression test for a bug found while porting
    VirtualFileSystem fs;
    fs.write_string("test_shader_vertex4.glsl", VALID_VERTEX_SRC);
    fs.write_string("test_shader_fragment4.glsl", VALID_FRAGMENT_SRC);

    Shader shader(fs.open("test_shader_vertex4.glsl"), fs.open("test_shader_fragment4.glsl"));
    shader.bind();
    unsigned int program = shader.get_shader_handle();

    shader.set_uniform("testUVec4", Vector4u{10, 20, 30, 40});
    unsigned int readUVec4[4] = {};
    glGetUniformuiv(program, shader.get_location("testUVec4"), readUVec4);
    EXPECT_EQ(readUVec4[0], 10u);
    EXPECT_EQ(readUVec4[1], 20u);
    EXPECT_EQ(readUVec4[2], 30u);
    EXPECT_EQ(readUVec4[3], 40u);

    fs.remove("test_shader_vertex4.glsl");
    fs.remove("test_shader_fragment4.glsl");
}

TEST_F(ShaderTest, ConstructionFromBrokenSourceThrowsInsteadOfExiting)
{
    VirtualFileSystem fs;
    fs.write_string("test_shader_vertex5.glsl", VALID_VERTEX_SRC);
    fs.write_string("test_shader_broken_fragment.glsl", BROKEN_FRAGMENT_SRC);

    EXPECT_THROW(
        Shader(fs.open("test_shader_vertex5.glsl"), fs.open("test_shader_broken_fragment.glsl")),
        std::runtime_error
    );

    fs.remove("test_shader_vertex5.glsl");
    fs.remove("test_shader_broken_fragment.glsl");
}
