#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/host_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    // Renderer is abstract (render_frame is pure virtual) - a trivial concrete subclass is
    // needed to construct one at all, matching the pattern already used for BufferedPass in
    // render_pass.test.cpp.
    class TestRenderer : public Renderer {
    public:
        using Renderer::Renderer;
        void render_frame(Time) override {}
    };

    const char* SOLID_COLOR_VERTEX_SRC =
        "#version 450 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aUV;\n"
        "void main(){\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "}\n";

    const char* SOLID_RED_FRAGMENT_SRC =
        "#version 450 core\n"
        "layout (location = 0) out vec4 outColor;\n"
        "void main(){\n"
        "    outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
}

class RendererTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "renderer_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* RendererTest::window = nullptr;

TEST_F(RendererTest, ConstructionForceAppliesTheDefaultState)
{
    TestRenderer renderer({64, 64});

    EXPECT_EQ(glIsEnabled(GL_DEPTH_TEST), GL_TRUE);
    EXPECT_EQ(glIsEnabled(GL_BLEND), GL_FALSE);
    EXPECT_EQ(glIsEnabled(GL_CULL_FACE), GL_FALSE);

    GLint frontFace = 0;
    glGetIntegerv(GL_FRONT_FACE, &frontFace);
    EXPECT_EQ(frontFace, GL_CCW);
}

TEST_F(RendererTest, SetStateAppliesBlendFuncAndEquationWhenBlendEnabled)
{
    TestRenderer renderer({64, 64});

    RenderState state;
    state.blend = true;
    state.blendSrc = GL_ONE;
    state.blendDst = GL_ZERO;
    state.blendEquation = GL_FUNC_SUBTRACT;
    renderer.set_state(state);

    EXPECT_EQ(glIsEnabled(GL_BLEND), GL_TRUE);

    GLint src = 0, dst = 0, eq = 0;
    glGetIntegerv(GL_BLEND_SRC_RGB, &src);
    glGetIntegerv(GL_BLEND_DST_RGB, &dst);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &eq);
    EXPECT_EQ(src, GL_ONE);
    EXPECT_EQ(dst, GL_ZERO);
    EXPECT_EQ(eq, GL_FUNC_SUBTRACT);
}

TEST_F(RendererTest, CullFaceModeOnlyUpdatesWhenCullingIsActuallyEnabled)
{
    TestRenderer renderer({64, 64});

    RenderState enabledFront;
    enabledFront.cullFace = true;
    enabledFront.cullMode = GL_FRONT;
    renderer.set_state(enabledFront);

    GLint mode = 0;
    glGetIntegerv(GL_CULL_FACE_MODE, &mode);
    EXPECT_EQ(mode, GL_FRONT);

    // Disable culling while also changing the requested mode
    RenderState disabledBack;
    disabledBack.cullFace = false;
    disabledBack.cullMode = GL_BACK;
    renderer.set_state(disabledBack);

    EXPECT_EQ(glIsEnabled(GL_CULL_FACE), GL_FALSE);
    glGetIntegerv(GL_CULL_FACE_MODE, &mode);
    EXPECT_EQ(mode, GL_FRONT); // Unchanged from before

    // Re-enabling with a genuinely different mode must update it for real.
    RenderState enabledBack;
    enabledBack.cullFace = true;
    enabledBack.cullMode = GL_BACK;
    renderer.set_state(enabledBack);

    glGetIntegerv(GL_CULL_FACE_MODE, &mode);
    EXPECT_EQ(mode, GL_BACK);
}

TEST_F(RendererTest, ViewportDefaultsToRenderSizeWhenUnset)
{
    TestRenderer renderer({32, 24});
    renderer.set_state(RenderState{});

    GLint viewport[4] = {};
    glGetIntegerv(GL_VIEWPORT, viewport);
    EXPECT_EQ(viewport[2], 32);
    EXPECT_EQ(viewport[3], 24);
}

TEST_F(RendererTest, ScissorAppliesTheGivenRect)
{
    TestRenderer renderer({64, 64});

    RenderState state;
    state.scissor = Rect<float>{2, 3, 10, 12};
    renderer.set_state(state);

    EXPECT_EQ(glIsEnabled(GL_SCISSOR_TEST), GL_TRUE);

    GLint box[4] = {};
    glGetIntegerv(GL_SCISSOR_BOX, box);
    EXPECT_EQ(box[0], 2);
    EXPECT_EQ(box[1], 3);
    EXPECT_EQ(box[2], 10);
    EXPECT_EQ(box[3], 12);
}

TEST_F(RendererTest, DrawFullscreenQuadCoversTheWholeViewport)
{
    TestRenderer renderer({8, 8});
    Framebuffer target({{8, 8}});

    HostFileSystem fs;
    fs.write_string("renderer_quad_v.glsl", SOLID_COLOR_VERTEX_SRC);
    fs.write_string("renderer_quad_f.glsl", SOLID_RED_FRAGMENT_SRC);
    Shader shader(fs.open("renderer_quad_v.glsl"), fs.open("renderer_quad_f.glsl"));

    target.begin({0, 0, 0, 1});
    shader.bind();
    renderer.draw_fullscreen_quad();

    unsigned char corners[4][4] = {};
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, corners[0]);
    glReadPixels(7, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, corners[1]);
    glReadPixels(0, 7, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, corners[2]);
    glReadPixels(7, 7, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, corners[3]);
    target.end();

    for(auto& pixel : corners){
        EXPECT_GT(pixel[0], 200);
        EXPECT_LT(pixel[1], 50);
    }
}

TEST_F(RendererTest, BeginEndPassTracksTheCurrentPass)
{
    TestRenderer renderer({64, 64});
    AbstractRenderPass pass;

    EXPECT_NO_THROW(renderer.begin_pass(pass));
    EXPECT_NO_THROW(renderer.end_pass());
}
