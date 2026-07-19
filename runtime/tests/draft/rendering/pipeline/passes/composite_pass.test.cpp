#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/pipeline/passes/composite_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/virtual_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    class TestRenderer : public Renderer {
    public:
        using Renderer::Renderer;
        void render_frame(Time, SystemRegistry&, const Camera&) override {}
    };

    const char* COMPOSITE_VERTEX_SRC =
        "#version 450 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aUV;\n"
        "out vec2 vUV;\n"
        "void main(){\n"
        "    vUV = aUV;\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "}\n";

    const char* COMPOSITE_FRAGMENT_SRC =
        "#version 450 core\n"
        "layout (location = 0) out vec4 outColor;\n"
        "in vec2 vUV;\n"
        "uniform sampler2D u_geometryTexture;\n"
        "void main(){\n"
        "    outColor = texture(u_geometryTexture, vUV);\n"
        "}\n";
}

class CompositePassTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "composite_pass_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* CompositePassTest::window = nullptr;

TEST_F(CompositePassTest, RunBlitsTheGeometryTextureOntoTheCurrentFramebuffer)
{
    TestRenderer renderer({8, 8});

    VirtualFileSystem fs;
    fs.write_string("composite_pass_v.glsl", COMPOSITE_VERTEX_SRC);
    fs.write_string("composite_pass_f.glsl", COMPOSITE_FRAGMENT_SRC);
    auto shader = std::make_shared<Shader>(fs.open("composite_pass_v.glsl"), fs.open("composite_pass_f.glsl"));
    Resource<Shader> shaderResource(std::make_shared<AssetSlot<Shader>>(std::move(shader)));

    // A solid-green source "geometry" texture, uploaded via a Framebuffer cleared to green.
    Framebuffer source({{8, 8}});
    source.begin({0.f, 1.f, 0.f, 1.f});
    source.end();

    Framebuffer destination({{8, 8}});
    destination.begin();

    CompositePass pass(shaderResource);
    glGetError();
    pass.run(renderer, source.get_texture());
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    unsigned char pixel[4] = {};
    glReadPixels(4, 4, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    destination.end();

    EXPECT_LT(pixel[0], 50);
    EXPECT_GT(pixel[1], 200);
    EXPECT_LT(pixel[2], 50);
}
