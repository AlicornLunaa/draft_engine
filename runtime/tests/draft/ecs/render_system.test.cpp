#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/ecs/render_system.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/host_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    // Renderer is abstract (render_frame is pure virtual). A trivial concrete subclass is
    // needed to construct one at all, matching every other pipeline test's TestRenderer.
    class TestRenderer : public Renderer {
    public:
        using Renderer::Renderer;
        void render_frame(Time) override {}
    };

    const char* SPRITE_VERTEX_SRC =
        "#version 450 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec4 aColor;\n"
        "layout (location = 2) in vec2 aTexCoord1;\n"
        "layout (location = 3) in vec2 aTexCoord2;\n"
        "layout (location = 4) in vec2 aTexCoord3;\n"
        "layout (location = 5) in vec2 aTexCoord4;\n"
        "layout(std430, binding=0) buffer Models { mat4 modelMatrices[]; };\n"
        "out vec2 vTexCoord;\n"
        "out vec4 vColor;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "vec2 aTexCoords[4] = vec2[4](aTexCoord1, aTexCoord2, aTexCoord3, aTexCoord4);\n"
        "void main(){\n"
        "    gl_Position = projection * view * modelMatrices[gl_InstanceID] * vec4(aPos.xy, 0.0, 1.0);\n"
        "    vTexCoord = aTexCoords[gl_VertexID];\n"
        "    vColor = aColor;\n"
        "}\n";

    const char* SPRITE_FRAGMENT_SRC =
        "#version 450 core\n"
        "layout (location = 0) out vec4 outColor;\n"
        "in vec2 vTexCoord;\n"
        "in vec4 vColor;\n"
        "uniform sampler2D baseTexture;\n"
        "void main(){\n"
        "    vec4 col = texture(baseTexture, vTexCoord);\n"
        "    if(col.a <= 0.0 || vColor.a == 0.0) discard;\n"
        "    outColor = col * vColor;\n"
        "}\n";
}

class RenderSystemTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "render_system_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    static Resource<Shader> make_shader(const std::string& vertName, const std::string& fragName){
        HostFileSystem fs;
        fs.write_string(vertName, SPRITE_VERTEX_SRC);
        fs.write_string(fragName, SPRITE_FRAGMENT_SRC);

        auto shader = std::make_shared<Shader>(fs.open(vertName), fs.open(fragName));
        return Resource<Shader>(std::make_shared<AssetSlot<Shader>>(std::move(shader)));
    }
};

RenderWindow* RenderSystemTest::window = nullptr;

TEST_F(RenderSystemTest, RenderSubmitsSpriteEntitiesIntoTheRendererBatchWithoutError)
{
    Scene scene;
    TestRenderer renderer({16, 16});

    scene.get_systems().add<RenderSystem>(scene.get_registry(), renderer);

    // SpriteComponent::shader overrides SpriteCollection's own process-lifetime leaked default
    Resource<Shader> shader = make_shader("render_system_v1.glsl", "render_system_f1.glsl");

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>(TransformComponent{{5.f, 7.f}, 0.f});
    SpriteComponent& sprite = entity.add_component<SpriteComponent>(Resource<Texture>{}, Vector2f{10.f, 10.f});
    sprite.shader = shader.get();

    scene.render(Time::seconds(0));

    glGetError();
    renderer.batch.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderSystemTest, RenderWithNoSpriteEntitiesDoesNotThrow)
{
    Scene scene;
    TestRenderer renderer({16, 16});

    scene.get_systems().add<RenderSystem>(scene.get_registry(), renderer);

    ASSERT_NO_THROW(scene.render(Time::seconds(0)));
}
