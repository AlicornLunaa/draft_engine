#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/pipeline/renderer.hpp"
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

class DefaultRendererTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "default_renderer_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    static Resource<Shader> make_shader(const std::string& vertName, const std::string& fragName){
        // TODO: Implement in-memory file system
        VirtualFileSystem fs;
        fs.write_string(vertName, SHAPES_VERTEX_SRC);
        fs.write_string(fragName, SHAPES_FRAGMENT_SRC);

        auto shader = std::make_shared<Shader>(fs.open(vertName), fs.open(fragName));
        return Resource<Shader>(std::make_shared<AssetSlot<Shader>>(std::move(shader)));
    }
};

RenderWindow* DefaultRendererTest::window = nullptr;

TEST_F(DefaultRendererTest, RenderFrameWithNothingSubmittedProducesNoGLError)
{
    DefaultRenderer renderer({16, 16});

    glGetError();
    renderer.render_frame(Time::seconds(0));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(DefaultRendererTest, RenderFrameRunsTheFullGeometryCompositeInterfaceChain)
{
    DefaultRenderer renderer({16, 16});

    renderer.shape.set_shader(make_shader("default_renderer_shape_v.glsl", "default_renderer_shape_f.glsl"));
    renderer.shape.draw_line({0, 0}, {1, 1});

    glGetError();
    renderer.render_frame(Time::seconds(0));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(DefaultRendererTest, ResizePropagatesToTheGeometryPass)
{
    DefaultRenderer renderer({16, 16});

    EXPECT_NO_THROW(renderer.resize({32, 24}));
    EXPECT_EQ(renderer.get_size().x, 32u);
    EXPECT_EQ(renderer.get_size().y, 24u);
}
