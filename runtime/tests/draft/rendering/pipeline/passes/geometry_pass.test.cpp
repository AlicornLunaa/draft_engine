#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/pipeline/passes/geometry_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/virtual_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    class TestRenderer : public Renderer {
    public:
        using Renderer::Renderer;
        void render_frame(Time) override {}
    };

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

class GeometryPassTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "geometry_pass_test");
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

RenderWindow* GeometryPassTest::window = nullptr;

TEST_F(GeometryPassTest, RunFlushesAlreadySubmittedGeometryAndReturnsARealTexture)
{
    TestRenderer renderer({16, 16});
    GeometryPass pass(make_shader("geometry_pass_v1.glsl", "geometry_pass_f1.glsl"), {16, 16});

    // Give renderer.shape a shader owned by this fixture's own GL context, rather than relying
    // on ShapeCollection's leaked default (which is only valid within whichever context first
    // constructed it)
    renderer.shape.set_shader(make_shader("geometry_pass_shape_v1.glsl", "geometry_pass_shape_f1.glsl"));

    // Simulate a RenderSystem having already submitted this frame's geometry before the pass
    // pipeline runs, GeometryPass itself no longer knows about Scene/ECS at all.
    renderer.shape.draw_line({0, 0}, {1, 1});

    glGetError();
    const Texture& result = pass.run(renderer);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
    EXPECT_NE(result.get_texture_handle(), 0u);
}

TEST_F(GeometryPassTest, RunDrainsTheSubmittedQueues)
{
    TestRenderer renderer({16, 16});
    GeometryPass pass(make_shader("geometry_pass_v2.glsl", "geometry_pass_f2.glsl"), {16, 16});

    renderer.shape.set_shader(make_shader("geometry_pass_shape_v2.glsl", "geometry_pass_shape_f2.glsl"));
    renderer.shape.draw_line({0, 0}, {1, 1});
    pass.run(renderer);

    // Nothing left to flush, a second flush should be a no-op producing no GL error.
    glGetError();
    renderer.shape.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GeometryPassTest, ResizePropagatesToTheUnderlyingFramebuffer)
{
    TestRenderer renderer({16, 16});
    GeometryPass pass(make_shader("geometry_pass_v3.glsl", "geometry_pass_f3.glsl"), {16, 16});

    pass.resize({32, 24});
    renderer.shape.set_shader(make_shader("geometry_pass_shape_v3.glsl", "geometry_pass_shape_f3.glsl"));
    renderer.shape.draw_line({0, 0}, {1, 1});
    const Texture& result = pass.run(renderer);

    EXPECT_EQ(result.get_properties().size.x, 32u);
    EXPECT_EQ(result.get_properties().size.y, 24u);
}
