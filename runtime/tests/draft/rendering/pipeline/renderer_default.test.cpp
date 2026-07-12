#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/ecs/system.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/virtual_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include <string>
#include <vector>

using namespace Draft;

namespace {
    // Records which layer(s) it was called for and in what order
    template<RenderLayer Layer>
    struct LayerRecordingSystem : AbstractSystem {
        std::vector<std::string>& log;
        explicit LayerRecordingSystem(std::vector<std::string>& log) : log(log) {}

        RenderLayer get_render_layers() const override { return Layer; }

        void render(Time, RenderLayer layer) override {
            if(layer == RenderLayer::Geometry) log.push_back("Geometry");
            else if(layer == RenderLayer::Interface) log.push_back("Interface");
            else if(layer == RenderLayer::Overlay) log.push_back("Overlay");
        }
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
    SystemRegistry systems;

    glGetError();
    renderer.render_frame(Time::seconds(0), systems);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(DefaultRendererTest, RenderFrameRunsTheFullGeometryCompositeInterfaceChain)
{
    DefaultRenderer renderer({16, 16});
    SystemRegistry systems;

    renderer.shape.set_shader(make_shader("default_renderer_shape_v.glsl", "default_renderer_shape_f.glsl"));
    renderer.shape.draw_line({0, 0}, {1, 1});

    glGetError();
    renderer.render_frame(Time::seconds(0), systems);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(DefaultRendererTest, ResizePropagatesToTheGeometryPass)
{
    DefaultRenderer renderer({16, 16});

    EXPECT_NO_THROW(renderer.resize({32, 24}));
    EXPECT_EQ(renderer.get_size().x, 32u);
    EXPECT_EQ(renderer.get_size().y, 24u);
}

TEST_F(DefaultRendererTest, RenderFrameDispatchesLayersInGeometryInterfaceOverlayOrder)
{
    DefaultRenderer renderer({16, 16});
    SystemRegistry systems;
    std::vector<std::string> log;

    // Registered in a deliberately "wrong" order (Overlay first) to confirm the pipeline's own
    // fixed ordering wins, not registration order.
    systems.add<LayerRecordingSystem<RenderLayer::Overlay>>(log);
    systems.add<LayerRecordingSystem<RenderLayer::Geometry>>(log);
    systems.add<LayerRecordingSystem<RenderLayer::Interface>>(log);

    renderer.render_frame(Time::seconds(0), systems);

    ASSERT_EQ(log.size(), 3u);
    EXPECT_EQ(log[0], "Geometry");
    EXPECT_EQ(log[1], "Interface");
    EXPECT_EQ(log[2], "Overlay");
}

TEST_F(DefaultRendererTest, RenderFrameOnlyCallsSystemsForLayersTheyDeclare)
{
    DefaultRenderer renderer({16, 16});
    SystemRegistry systems;
    std::vector<std::string> log;

    systems.add<LayerRecordingSystem<RenderLayer::Geometry>>(log);

    renderer.render_frame(Time::seconds(0), systems);

    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0], "Geometry");
}
