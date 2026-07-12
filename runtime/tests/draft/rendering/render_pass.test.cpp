#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/pipeline/render_pass.hpp"
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

    const char* VALID_FRAGMENT_SRC =
        "#version 450 core\n"
        "out vec4 outColor;\n"
        "void main(){\n"
        "    outColor = vec4(1.0);\n"
        "}\n";

    // Exposes BufferedPass's protected members so tests can assert on real state without adding
    // test-only accessors to the production class itself.
    class TestBufferedPass : public BufferedPass {
    public:
        using BufferedPass::BufferedPass;
        using BufferedPass::p_frameBuffer;
        using BufferedPass::p_shader;
        using BufferedPass::p_state;
    };
}

class RenderPassTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "render_pass_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    static Resource<Shader> make_shader(const std::string& vertName, const std::string& fragName){
        VirtualFileSystem fs;
        fs.write_string(vertName, VALID_VERTEX_SRC);
        fs.write_string(fragName, VALID_FRAGMENT_SRC);

        auto shader = std::make_shared<Shader>(fs.open(vertName), fs.open(fragName));
        return Resource<Shader>(std::make_shared<AssetSlot<Shader>>(std::move(shader)));
    }
};

RenderWindow* RenderPassTest::window = nullptr;

TEST_F(RenderPassTest, ConstructionCreatesAFramebufferAtTheRequestedSize)
{
    Resource<Shader> shader = make_shader("render_pass_test_vertex1.glsl", "render_pass_test_fragment1.glsl");
    TestBufferedPass pass(shader, {16, 12});

    EXPECT_EQ(pass.p_frameBuffer.get_size().x, 16u);
    EXPECT_EQ(pass.p_frameBuffer.get_size().y, 12u);
    EXPECT_TRUE(pass.p_shader.is_valid());
}

TEST_F(RenderPassTest, ResizePropagatesToTheFramebuffer)
{
    Resource<Shader> shader = make_shader("render_pass_test_vertex2.glsl", "render_pass_test_fragment2.glsl");
    TestBufferedPass pass(shader, {8, 8});

    pass.resize({64, 48});

    EXPECT_EQ(pass.p_frameBuffer.get_size().x, 64u);
    EXPECT_EQ(pass.p_frameBuffer.get_size().y, 48u);
}
