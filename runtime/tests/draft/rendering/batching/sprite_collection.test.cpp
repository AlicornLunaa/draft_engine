#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/virtual_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    // Mirrors assets/shaders/default/{vertex,fragment}.glsl binding=0 SSBO of model matrices,
    // attribute locations 0-5, a baseTexture sampler, and view/projection uniforms.
    const char* DEFAULT_VERTEX_SRC =
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

    const char* DEFAULT_FRAGMENT_SRC =
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

// SpriteCollection's constructor issues real GL calls, so the whole suite shares one hidden
// RenderWindow/GL context instead of creating one per test.
class SpriteCollectionTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "sprite_collection_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    static Resource<Shader> make_shader(const std::string& vertName, const std::string& fragName){
        // TODO: Implement in-memory file system
        VirtualFileSystem fs;
        fs.write_string(vertName, DEFAULT_VERTEX_SRC);
        fs.write_string(fragName, DEFAULT_FRAGMENT_SRC);

        auto shader = std::make_shared<Shader>(fs.open(vertName), fs.open(fragName));
        return Resource<Shader>(std::make_shared<AssetSlot<Shader>>(std::move(shader)));
    }
};

RenderWindow* SpriteCollectionTest::window = nullptr;

TEST_F(SpriteCollectionTest, DrawOpaqueWithExplicitMaterialThenFlushProducesNoGLError)
{
    SpriteCollection collection;
    Resource<Shader> shader = make_shader("sprite_v1.glsl", "sprite_f1.glsl");
    Texture texture(Image({2, 2}, {1.f, 0.f, 0.f, 1.f}, ColorFormat::RGB));

    SpriteProps props;
    props.material.shader = shader.get();
    props.material.baseTexture = &texture;
    props.material.tint = {1, 1, 1, 1};
    props.material.transparent = false;

    collection.draw(props);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(SpriteCollectionTest, DrawWithUnsetMaterialResolvesToRealDefaultsAndFlushesCleanly)
{
    SpriteCollection collection;

    SpriteProps props; // shader/baseTexture/normalTexture/emissiveTexture all left null
    collection.draw(props);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(SpriteCollectionTest, TwoCollectionsDefaultDrawsShareTheSameShaderProgram)
{
    SpriteCollection first;
    first.draw(SpriteProps{});
    first.flush();

    GLint firstProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &firstProgram);
    ASSERT_NE(firstProgram, 0);

    glUseProgram(0);

    SpriteCollection second;
    second.draw(SpriteProps{});
    second.flush();

    GLint secondProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &secondProgram);

    EXPECT_EQ(firstProgram, secondProgram);
}

TEST_F(SpriteCollectionTest, DrawTranslucentSpriteRoutesThroughTransparentPathWithNoGLError)
{
    SpriteCollection collection;

    SpriteProps props;
    props.material.tint = {1, 1, 1, 0.5f}; // alpha < 1 -> transparent queue

    collection.draw(props);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(SpriteCollectionTest, MixedOpaqueAndTransparentDrawsInOneFlushProduceNoGLError)
{
    SpriteCollection collection;

    SpriteProps opaque;
    collection.draw(opaque);

    SpriteProps transparent;
    transparent.material.transparent = true;
    collection.draw(transparent);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(SpriteCollectionTest, DrawingMoreThanOneChunkOfSpritesProducesNoGLError)
{
    SpriteCollection collection;

    // MAX_SPRITES_TO_RENDER is 1024, force the chunking loop in flush_generic() to wrap at least once.
    for(size_t i = 0; i < SpriteCollection::MAX_SPRITES_TO_RENDER + 500; i++){
        SpriteProps props;
        props.position = {static_cast<float>(i), 0.f};
        collection.draw(props);
    }

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
