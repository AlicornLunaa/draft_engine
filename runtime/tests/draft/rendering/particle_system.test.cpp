#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/particle_system.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/host_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    // Mirrors assets/shaders/default/{vertex,fragment}.glsl, binding=0 SSBO of model matrices,
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

    Resource<Shader> make_shader(const std::string& vertName, const std::string& fragName){
        HostFileSystem fs;
        fs.write_string(vertName, DEFAULT_VERTEX_SRC);
        fs.write_string(fragName, DEFAULT_FRAGMENT_SRC);

        auto shader = std::make_shared<Shader>(fs.open(vertName), fs.open(fragName));
        return Resource<Shader>(std::make_shared<AssetSlot<Shader>>(std::move(shader)));
    }
}

namespace Draft {
    struct ParticleSystemTestAccess {
        static ParticleSystem::VisualState compute_visual_state(float startSize, float endSize, const Vector4f& colorBegin, const Vector4f& colorEnd, float lifeFraction){
            return ParticleSystem::compute_visual_state(startSize, endSize, colorBegin, colorEnd, lifeFraction);
        }

        static size_t recycle_index(size_t poolIndex, size_t poolSize){
            return ParticleSystem::recycle_index(poolIndex, poolSize);
        }
    };
}

// Pure math, no GL needed - directly pins the begin/end interpolation direction fix.
TEST(ParticleSystemVisualState, AtBirthUsesTheBeginValuesUnfaded)
{
    // lifeFraction = 1 at birth (full life remaining).
    auto state = ParticleSystemTestAccess::compute_visual_state(/*startSize=*/2.f, /*endSize=*/8.f, /*colorBegin=*/{1, 0, 0, 1}, /*colorEnd=*/{0, 0, 1, 1}, /*lifeFraction=*/1.f);

    EXPECT_FLOAT_EQ(state.size, 2.f);
    EXPECT_FLOAT_EQ(state.color.r, 1.f);
    EXPECT_FLOAT_EQ(state.color.b, 0.f);
    EXPECT_FLOAT_EQ(state.color.a, 1.f); // unfaded at birth
}

TEST(ParticleSystemVisualState, AtDeathUsesTheEndValuesFullyFaded)
{
    // lifeFraction = 0 at death (no life remaining).
    auto state = ParticleSystemTestAccess::compute_visual_state(2.f, 8.f, {1, 0, 0, 1}, {0, 0, 1, 1}, 0.f);

    EXPECT_FLOAT_EQ(state.size, 8.f);
    EXPECT_FLOAT_EQ(state.color.r, 0.f);
    EXPECT_FLOAT_EQ(state.color.b, 1.f);
    EXPECT_FLOAT_EQ(state.color.a, 0.f); // fully faded at death
}

// Pure math, no GL needed. Directly pins the pool-recycling wraparound fix.
TEST(ParticleSystemRecycleIndex, DecrementsNormallyAwayFromZero)
{
    EXPECT_EQ(ParticleSystemTestAccess::recycle_index(5, 10), 4u);
}

TEST(ParticleSystemRecycleIndex, WrapsCleanlyToTheLastSlotFromZero)
{
    // The old `--poolIndex % poolSize` idiom underflows poolIndex to SIZE_MAX before the modulo
    // when poolIndex is 0, landing on SIZE_MAX % poolSize (615 for poolSize=1000), not
    // poolSize - 1.
    EXPECT_EQ(ParticleSystemTestAccess::recycle_index(0, 1000), 999u);
    EXPECT_NE(ParticleSystemTestAccess::recycle_index(0, 1000), (size_t)(SIZE_MAX % 1000));
}

TEST(ParticleSystemRecycleIndex, HandlesAPoolOfSizeOne)
{
    EXPECT_EQ(ParticleSystemTestAccess::recycle_index(0, 1), 0u);
}

class ParticleSystemTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "particle_system_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* ParticleSystemTest::window = nullptr;

TEST_F(ParticleSystemTest, EmitUpdateRenderFlushProducesNoGLError)
{
    ParticleSystem particles(16);
    Resource<Shader> shader = make_shader("particle_v1.glsl", "particle_f1.glsl");

    ParticleProps props;
    props.position = {5.f, 5.f};
    props.lifeTime = 1.f;
    props.shader = shader.get();
    particles.emit(props);
    particles.emit(props);

    particles.update(Time::seconds(0.1f));

    SpriteCollection collection;
    particles.render(collection);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ParticleSystemTest, ParticlesThatAgeOutStopRenderingWithoutError)
{
    ParticleSystem particles(4);
    Resource<Shader> shader = make_shader("particle_v2.glsl", "particle_f2.glsl");

    ParticleProps props;
    props.lifeTime = 0.05f;
    props.shader = shader.get();
    particles.emit(props);

    // Advance well past lifeTime, the particle should become inactive and render() should
    // simply skip it, not error or crash.
    particles.update(Time::seconds(1.f));

    SpriteCollection collection;
    particles.render(collection);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ParticleSystemTest, EmittingMoreThanThePoolSizeRecyclesWithoutError)
{
    ParticleSystem particles(4);
    Resource<Shader> shader = make_shader("particle_v3.glsl", "particle_f3.glsl");

    ParticleProps props;
    props.lifeTime = 10.f; // stay alive across every emit below
    props.shader = shader.get();
    for(int i = 0; i < 10; i++){
        props.position = {(float)i, 0.f};
        particles.emit(props);
    }

    SpriteCollection collection;
    particles.render(collection);

    glGetError();
    collection.flush();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
