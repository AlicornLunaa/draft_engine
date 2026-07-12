#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/batching/shape_collection.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/virtual_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    const char* SHAPES_VERTEX_SRC =
        "#version 450 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec4 aColor;\n"
        "out vec4 vColor;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "uniform float zLayer = 0.f;\n"
        "void main(){\n"
        "    gl_Position = projection * view * vec4(aPos.xy, zLayer, 1.0);\n"
        "    vColor = aColor;\n"
        "}\n";

    const char* SHAPES_FRAGMENT_SRC =
        "#version 450 core\n"
        "layout (location = 0) out vec4 outColor;\n"
        "in vec4 vColor;\n"
        "void main(){\n"
        "    outColor = vColor;\n"
        "}\n";
}

// ShapeCollection's constructor (even default-constructed) issues real GL calls, so the whole
// suite shares one hidden RenderWindow/GL context instead of creating one per test.
class ShapeCollectionTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "shape_collection_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    static Resource<Shader> make_shader(const std::string& vertName, const std::string& fragName){
        VirtualFileSystem fs;
        fs.write_string(vertName, SHAPES_VERTEX_SRC);
        fs.write_string(fragName, SHAPES_FRAGMENT_SRC);

        auto shader = std::make_shared<Shader>(fs.open(vertName), fs.open(fragName));
        return Resource<Shader>(std::make_shared<AssetSlot<Shader>>(std::move(shader)));
    }
};

RenderWindow* ShapeCollectionTest::window = nullptr;

TEST_F(ShapeCollectionTest, DefaultConstructionLoadsTheRealEmbeddedDefaultShader)
{
    ShapeCollection collection;
    EXPECT_NE(collection.get_shader().get_shader_handle(), 0u);
}

TEST_F(ShapeCollectionTest, TwoDefaultConstructedCollectionsShareTheSameDefaultShader)
{
    ShapeCollection first;
    ShapeCollection second;
    EXPECT_EQ(first.get_shader().get_shader_handle(), second.get_shader().get_shader_handle());
}

TEST_F(ShapeCollectionTest, DrawLineThenFlushProducesNoGLError)
{
    ShapeCollection collection(make_shader("shape_v1.glsl", "shape_f1.glsl"));

    collection.draw_line({0, 0}, {1, 1});
    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ShapeCollectionTest, DrawRectInLineAndFillModesProducesNoGLError)
{
    ShapeCollection collection(make_shader("shape_v2.glsl", "shape_f2.glsl"));

    collection.set_render_type(ShapeRenderType::LINE);
    collection.draw_rect({0, 0}, {1, 1}, 0.f);

    collection.set_render_type(ShapeRenderType::FILL);
    collection.draw_rect({2, 2}, {1, 1}, 0.f);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ShapeCollectionTest, DrawCircleInLineAndFillModesProducesNoGLError)
{
    ShapeCollection collection(make_shader("shape_v3.glsl", "shape_f3.glsl"));

    collection.set_render_type(ShapeRenderType::LINE);
    collection.draw_circle({0, 0}, 1.f, 0.f, 12);

    collection.set_render_type(ShapeRenderType::FILL);
    collection.draw_circle({2, 2}, 1.f, 0.f, 12);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ShapeCollectionTest, DrawTriangleOverloadsProduceNoGLError)
{
    ShapeCollection collection(make_shader("shape_v4.glsl", "shape_f4.glsl"));

    collection.set_render_type(ShapeRenderType::FILL);
    collection.draw_triangle({0, 0}, {1, 1}, 0.f);
    collection.draw_triangle(std::array<Vector2f, 3>{Vector2f{0, 0}, Vector2f{1, 0}, Vector2f{0, 1}});

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ShapeCollectionTest, DrawPolygonProducesNoGLError)
{
    ShapeCollection collection(make_shader("shape_v5.glsl", "shape_f5.glsl"));

    std::vector<Vector2f> verts{{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    collection.draw_polygon(verts);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ShapeCollectionTest, DrawRectLineAndDottedLineProduceNoGLError)
{
    ShapeCollection collection(make_shader("shape_v6.glsl", "shape_f6.glsl"));

    collection.draw_rect_line({0, 0}, {5, 0}, 1.f);
    collection.draw_dotted_line({0, 1}, {5, 1}, 1.f, 0.5f);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ShapeCollectionTest, DrawArrowProducesNoGLError)
{
    ShapeCollection collection(make_shader("shape_v7.glsl", "shape_f7.glsl"));

    collection.draw_arrow({1, 1}, {0, 0}, 1.f);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ShapeCollectionTest, ColorZLayerAndRenderTypeRoundTrip)
{
    ShapeCollection collection(make_shader("shape_v8.glsl", "shape_f8.glsl"));

    collection.set_color({0.1f, 0.2f, 0.3f, 0.4f});
    EXPECT_FLOAT_EQ(collection.get_color().r, 0.1f);
    EXPECT_FLOAT_EQ(collection.get_color().a, 0.4f);

    collection.set_z_layer(5.f);
    EXPECT_FLOAT_EQ(collection.get_z_layer(), 5.f);

    collection.set_render_type(ShapeRenderType::FILL);
    EXPECT_EQ(collection.get_render_type(), ShapeRenderType::FILL);
}

TEST_F(ShapeCollectionTest, SetShaderChangesTheActiveShaderAndReboundsOnFlush)
{
    ShapeCollection collection(make_shader("shape_v9.glsl", "shape_f9.glsl"));
    Resource<Shader> replacement = make_shader("shape_v10.glsl", "shape_f10.glsl");

    EXPECT_NE(collection.get_shader().get_shader_handle(), replacement->get_shader_handle());

    collection.set_shader(replacement);
    EXPECT_EQ(collection.get_shader().get_shader_handle(), replacement->get_shader_handle());

    collection.draw_line({0, 0}, {1, 1});
    collection.flush();

    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    EXPECT_EQ(static_cast<unsigned int>(currentProgram), replacement->get_shader_handle());
}

TEST_F(ShapeCollectionTest, DrawLineWhileFillAutoSwitchesToLineRenderType)
{
    ShapeCollection collection(make_shader("shape_v11.glsl", "shape_f11.glsl"));

    collection.set_render_type(ShapeRenderType::FILL);
    collection.draw_line({0, 0}, {1, 1});

    EXPECT_EQ(collection.get_render_type(), ShapeRenderType::LINE);
}
