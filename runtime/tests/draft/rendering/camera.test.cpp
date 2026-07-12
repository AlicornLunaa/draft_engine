#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/camera.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/virtual_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

TEST(Camera, OrthographicGettersMatchConstructorArgs)
{
    OrthographicCamera cam({0, 0, 0}, {0, 0, -1}, -10.f, 10.f, -5.f, 5.f, 0.1f, 100.f);
    EXPECT_FLOAT_EQ(cam.get_left(), -10.f);
    EXPECT_FLOAT_EQ(cam.get_right(), 10.f);
    EXPECT_FLOAT_EQ(cam.get_bottom(), -5.f);
    EXPECT_FLOAT_EQ(cam.get_top(), 5.f);
    EXPECT_FLOAT_EQ(cam.get_near(), 0.1f);
    EXPECT_FLOAT_EQ(cam.get_far(), 100.f);
    EXPECT_FLOAT_EQ(cam.get_zoom(), 1.f);
}

TEST(Camera, SetZoomChangesTheProjectionMatrix)
{
    OrthographicCamera cam({0, 0, 0}, {0, 0, -1}, -10.f, 10.f, -5.f, 5.f, 0.1f, 100.f);
    Matrix4 before = cam.get_projection();

    cam.set_zoom(2.f);
    Matrix4 after = cam.get_projection();

    EXPECT_FLOAT_EQ(cam.get_zoom(), 2.f);
    EXPECT_NE(before[0][0], after[0][0]);
}

TEST(Camera, PerspectiveAspectRatioComesFromViewportSize)
{
    // Aspect ratio isn't directly exposed, but a 2x1 viewport should produce a visibly different
    // projection matrix than a 1x1 one (the x-scale term, projMatrix[0][0], is aspect-dependent).
    PerspectiveCamera square({0, 0, 0}, {0, 0, -1}, {100, 100});
    PerspectiveCamera wide({0, 0, 0}, {0, 0, -1}, {200, 100});

    EXPECT_NE(square.get_projection()[0][0], wide.get_projection()[0][0]);
}

TEST(Camera, TargetPointsForwardAtTheGivenPosition)
{
    Camera* cam = new OrthographicCamera({0, 0, 0}, {0, 0, -1}, -1.f, 1.f, -1.f, 1.f);
    cam->target({10, 0, 0});

    Vector3f forward = cam->get_forward();
    EXPECT_GT(forward.x, 0.9f); // should now point roughly toward +x

    delete cam;
}

TEST(Camera, GetCombinedIsProjectionTimesView)
{
    OrthographicCamera cam({1, 2, 3}, {0, 0, -1}, -1.f, 1.f, -1.f, 1.f);
    Matrix4 expected = cam.get_projection() * cam.get_view();
    Matrix4 combined = cam.get_combined();

    for(int col = 0; col < 4; col++){
        for(int row = 0; row < 4; row++){
            EXPECT_FLOAT_EQ(combined[col][row], expected[col][row]);
        }
    }
}

// Camera itself has no GL dependency except apply(), which just delegates to Shader::set_uniform
// and so needs a live GL context + a real compiled shader.
class CameraApplyTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "camera_apply_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* CameraApplyTest::window = nullptr;

TEST_F(CameraApplyTest, ApplyPushesViewAndProjectionUniforms)
{
    VirtualFileSystem fs;
    fs.write_string("test_camera_vertex.glsl",
        "#version 450 core\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main(){ gl_Position = projection * view * vec4(0.0, 0.0, 0.0, 1.0); }\n");
    fs.write_string("test_camera_fragment.glsl",
        "#version 450 core\n"
        "out vec4 outColor;\n"
        "void main(){ outColor = vec4(1.0); }\n");

    Shader shader(fs.open("test_camera_vertex.glsl"), fs.open("test_camera_fragment.glsl"));
    shader.bind();

    OrthographicCamera cam({5, 0, 0}, {0, 0, -1}, -1.f, 1.f, -1.f, 1.f);
    cam.apply(shader);

    unsigned int program = shader.get_shader_handle();
    float readView[16] = {};
    glGetUniformfv(program, shader.get_location("view"), readView);

    const Matrix4& expected = cam.get_view();
    for(int i = 0; i < 4; i++){
        EXPECT_FLOAT_EQ(readView[i], expected[0][i]);
    }

    fs.remove("test_camera_vertex.glsl");
    fs.remove("test_camera_fragment.glsl");
}
