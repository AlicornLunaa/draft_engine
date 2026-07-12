#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/material.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/host_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    const char* VALID_VERTEX_SRC =
        "#version 450 core\n"
        "void main(){\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    // References every uniform Material2D/Material3D::apply() touch so the compiler can't
    // optimize any of them away.
    const char* VALID_FRAGMENT_SRC =
        "#version 450 core\n"
        "out vec4 outColor;\n"
        "uniform sampler2D baseTexture;\n"
        "uniform sampler2D normalTexture;\n"
        "uniform sampler2D emissiveTexture;\n"
        "struct Material {\n"
        "    vec4 tint;\n"
        "    vec4 baseColor;\n"
        "    vec3 emissiveFactor;\n"
        "    float metallicFactor;\n"
        "    float roughnessFactor;\n"
        "    float normalScale;\n"
        "    float occlusionStrength;\n"
        "    sampler2D baseTexture;\n"
        "    sampler2D normalTexture;\n"
        "    sampler2D emissiveTexture;\n"
        "    sampler2D occlusionTexture;\n"
        "    sampler2D roughnessTexture;\n"
        "};\n"
        "uniform Material material;\n"
        "uniform float testUniformFloat;\n"
        "void main(){\n"
        "    outColor = texture(baseTexture, vec2(0.0)) + texture(normalTexture, vec2(0.0)) + texture(emissiveTexture, vec2(0.0))\n"
        "        + material.tint + material.baseColor + vec4(material.emissiveFactor, 0.0)\n"
        "        + vec4(material.metallicFactor + material.roughnessFactor + material.normalScale + material.occlusionStrength)\n"
        "        + texture(material.baseTexture, vec2(0.0)) + texture(material.normalTexture, vec2(0.0))\n"
        "        + texture(material.emissiveTexture, vec2(0.0)) + texture(material.occlusionTexture, vec2(0.0))\n"
        "        + texture(material.roughnessTexture, vec2(0.0)) + vec4(testUniformFloat);\n"
        "}\n";
}

class MaterialTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "material_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    static Shader make_shader(const std::string& vertName, const std::string& fragName){
        HostFileSystem fs;
        fs.write_string(vertName, VALID_VERTEX_SRC);
        fs.write_string(fragName, VALID_FRAGMENT_SRC);
        return Shader(fs.open(vertName), fs.open(fragName));
    }

    static unsigned int bound_texture_at_unit(unsigned int unit){
        glActiveTexture(GL_TEXTURE0 + unit);
        GLint bound = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound);
        return static_cast<unsigned int>(bound);
    }
};

RenderWindow* MaterialTest::window = nullptr;

TEST_F(MaterialTest, Material3DApplyBindsExplicitTexturesToTheirUnits)
{
    Shader shader = make_shader("material3d_v1.glsl", "material3d_f1.glsl");

    Texture base, normal, emissive, occlusion, roughness;
    Material3D material("test");
    material.baseTexture = &base;
    material.normalTexture = &normal;
    material.emissiveTexture = &emissive;
    material.occlusionTexture = &occlusion;
    material.roughnessTexture = &roughness;

    material.apply(shader);

    EXPECT_EQ(bound_texture_at_unit(0), base.get_texture_handle());
    EXPECT_EQ(bound_texture_at_unit(1), normal.get_texture_handle());
    EXPECT_EQ(bound_texture_at_unit(2), emissive.get_texture_handle());
    EXPECT_EQ(bound_texture_at_unit(3), occlusion.get_texture_handle());
    EXPECT_EQ(bound_texture_at_unit(4), roughness.get_texture_handle());
}

TEST_F(MaterialTest, Material3DApplyFallsBackToASharedDebugTextureWhenSlotsAreNull)
{
    Shader shader = make_shader("material3d_v2.glsl", "material3d_f2.glsl");

    Material3D warmup("warmup");
    warmup.apply(shader);

    Material3D first("first");
    first.apply(shader);
    unsigned int firstFallback = bound_texture_at_unit(0);
    EXPECT_NE(firstFallback, 0u);

    Material3D second("second");
    second.apply(shader);
    unsigned int secondFallback = bound_texture_at_unit(0);

    EXPECT_EQ(firstFallback, secondFallback);
}

TEST_F(MaterialTest, Material2DApplyIsANoOpWithoutAShader)
{
    Material2D material;
    EXPECT_NO_THROW(material.apply());
}

TEST_F(MaterialTest, Material2DApplyBindsTexturesAndSetsTint)
{
    Shader shader = make_shader("material2d_v1.glsl", "material2d_f1.glsl");

    Texture base, normal, emissive;
    Material2D material;
    material.shader = &shader;
    material.baseTexture = &base;
    material.normalTexture = &normal;
    material.emissiveTexture = &emissive;
    material.tint = {0.25f, 0.5f, 0.75f, 1.f};

    material.apply();

    EXPECT_EQ(bound_texture_at_unit(0), base.get_texture_handle());
    EXPECT_EQ(bound_texture_at_unit(1), normal.get_texture_handle());
    EXPECT_EQ(bound_texture_at_unit(2), emissive.get_texture_handle());

    float tint[4] = {};
    glGetUniformfv(shader.get_shader_handle(), shader.get_location("material.tint"), tint);
    EXPECT_FLOAT_EQ(tint[0], 0.25f);
    EXPECT_FLOAT_EQ(tint[1], 0.5f);
    EXPECT_FLOAT_EQ(tint[2], 0.75f);
}

TEST_F(MaterialTest, Material2DApplyUniformsRoundTripsCustomUniforms)
{
    Shader shader = make_shader("material2d_v2.glsl", "material2d_f2.glsl");

    Material2D material;
    material.shader = &shader;
    material.uniforms["testUniformFloat"] = 42.f;

    // apply_uniforms() (unlike apply()) doesn't bind the shader itself
    shader.bind();
    material.apply_uniforms();

    float value = 0.f;
    glGetUniformfv(shader.get_shader_handle(), shader.get_location("testUniformFloat"), &value);
    EXPECT_FLOAT_EQ(value, 42.f);
}

TEST_F(MaterialTest, Material2DEqualityComparesByFieldsAndPointerIdentity)
{
    Texture base;
    Material2D a;
    a.name = "mat";
    a.baseTexture = &base;
    a.tint = {1, 0, 0, 1};

    Material2D b = a;
    EXPECT_TRUE(a == b);

    b.tint = {0, 1, 0, 1};
    EXPECT_FALSE(a == b);
}
