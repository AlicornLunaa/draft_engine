#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/model.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/files/host_file_system.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include <cmath>
#include <cstdint>

using namespace Draft;

namespace Draft {
    struct ModelTestAccess {
        static Matrix4 compute_local_matrix(const std::vector<double>& translation, const std::vector<double>& rotation, const std::vector<double>& scale, const std::vector<double>& matrix){
            return Model::compute_local_matrix(translation, rotation, scale, matrix);
        }

        static std::vector<int> read_indices(const unsigned char* data, int componentType, size_t count){
            return Model::read_indices(data, componentType, count);
        }

        static size_t mesh_count(const Model& model){ return model.meshes.size(); }
        static const Matrix4& mesh_matrix(const Model& model, size_t i){ return model.meshToMatrixMap[i]; }
        static int mesh_material_index(const Model& model, size_t i){ return model.meshToMaterialMap[i]; }
        static size_t embedded_texture_count(const Model& model){ return model.embeddedTextures.size(); }
    };
}

TEST(ModelLocalMatrix, EmptyTRSAndMatrixIsIdentity)
{
    Matrix4 m = ModelTestAccess::compute_local_matrix({}, {}, {}, {});
    EXPECT_FLOAT_EQ(m[3].x, 0.f);
    EXPECT_FLOAT_EQ(m[3].y, 0.f);
    EXPECT_FLOAT_EQ(m[3].z, 0.f);
}

TEST(ModelLocalMatrix, TranslationOnlySetsTheTranslationColumn)
{
    Matrix4 m = ModelTestAccess::compute_local_matrix({2.0, 3.0, 4.0}, {}, {}, {});
    EXPECT_FLOAT_EQ(m[3].x, 2.f);
    EXPECT_FLOAT_EQ(m[3].y, 3.f);
    EXPECT_FLOAT_EQ(m[3].z, 4.f);
}

TEST(ModelLocalMatrix, QuaternionRotationIsNotTreatedAsEulerAngles)
{
    double s = std::sin(M_PI / 4.0);
    double c = std::cos(M_PI / 4.0);
    Matrix4 m = ModelTestAccess::compute_local_matrix({}, {0.0, 0.0, s, c}, {}, {});

    Vector4f rotated = m * Vector4f{1.f, 0.f, 0.f, 1.f};
    EXPECT_NEAR(rotated.x, 0.f, 1e-5f);
    EXPECT_NEAR(rotated.y, 1.f, 1e-5f);
    EXPECT_NEAR(rotated.z, 0.f, 1e-5f);
}

TEST(ModelLocalMatrix, RawMatrixTakesPrecedenceOverTRS)
{
    // glTF nodes specify *either* matrix *or* T/R/S, never both
    std::vector<double> matrix = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        5,6,7,1
    };
    Matrix4 m = ModelTestAccess::compute_local_matrix({99.0, 99.0, 99.0}, {}, {}, matrix);

    EXPECT_FLOAT_EQ(m[3].x, 5.f);
    EXPECT_FLOAT_EQ(m[3].y, 6.f);
    EXPECT_FLOAT_EQ(m[3].z, 7.f);
}

TEST(ModelReadIndices, ReadsUnsignedByteIndices)
{
    unsigned char bytes[] = { 0, 1, 2, 255 };
    auto indices = ModelTestAccess::read_indices(bytes, GL_UNSIGNED_BYTE, 4);
    ASSERT_EQ(indices.size(), 4u);
    EXPECT_EQ(indices[3], 255);
}

TEST(ModelReadIndices, ReadsUnsignedShortIndices)
{
    uint16_t values[] = { 0, 1, 2, 65535 };
    auto indices = ModelTestAccess::read_indices(reinterpret_cast<const unsigned char*>(values), GL_UNSIGNED_SHORT, 4);
    ASSERT_EQ(indices.size(), 4u);
    EXPECT_EQ(indices[3], 65535);
}

TEST(ModelReadIndices, ReadsUnsignedIntIndicesBeyondUnsignedShortRange)
{
    uint32_t values[] = { 0, 1, 100000 };
    auto indices = ModelTestAccess::read_indices(reinterpret_cast<const unsigned char*>(values), GL_UNSIGNED_INT, 3);
    ASSERT_EQ(indices.size(), 3u);
    EXPECT_EQ(indices[2], 100000);
}

namespace {
    const std::vector<float> TRIANGLE_POSITIONS = {
        0.f, 0.f, 0.f,
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f
    };

    void write_bin(HostFileSystem& fs, const std::string& binName, const void* data, size_t byteLength){
        fs.write_bytes(binName, data, byteLength);
    }
}

TEST(ModelLoad, SingleNonIndexedTriangleLoadsWithOneFlattenedPrimitive)
{
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    RenderWindow window(64, 64, "model_test_triangle");

    HostFileSystem fs;
    write_bin(fs, "model_triangle.bin", TRIANGLE_POSITIONS.data(), TRIANGLE_POSITIONS.size() * sizeof(float));

    const std::string json = R"({
        "asset": {"version": "2.0"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0}],
        "meshes": [{"primitives": [{"attributes": {"POSITION": 0}, "mode": 4}]}],
        "accessors": [
            {"bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3", "min": [0,0,0], "max": [1,1,0]}
        ],
        "bufferViews": [
            {"buffer": 0, "byteOffset": 0, "byteLength": 36}
        ],
        "buffers": [
            {"uri": "model_triangle.bin", "byteLength": 36}
        ]
    })";
    fs.write_string("model_triangle.gltf", json);

    Model model(fs.open("model_triangle.gltf"));
    fs.remove("model_triangle.bin");
    fs.remove("model_triangle.gltf");

    EXPECT_EQ(ModelTestAccess::mesh_count(model), 1u);
}

TEST(ModelLoad, IndexedQuadWithMissingTexcoordLoadsWithoutError)
{
    // TEXCOORD_0 is optional per the glTF spec this primitive has none, pinning finding #2's
    // guard (the old engine indexed primitive.attributes["TEXCOORD_0"] unconditionally).
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    RenderWindow window(64, 64, "model_test_quad");

    HostFileSystem fs;
    std::vector<float> positions = {
        0.f, 0.f, 0.f,
        1.f, 0.f, 0.f,
        1.f, 1.f, 0.f,
        0.f, 1.f, 0.f
    };
    std::vector<uint16_t> indices = { 0, 1, 2, 0, 2, 3 };

    fs.write_bytes("model_quad.bin", positions.data(), positions.size() * sizeof(float));
    fs.write_bytes("model_quad_indices.bin", indices.data(), indices.size() * sizeof(uint16_t));

    const std::string json = R"({
        "asset": {"version": "2.0"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0}],
        "meshes": [{"primitives": [{"attributes": {"POSITION": 0}, "indices": 1, "mode": 4}]}],
        "accessors": [
            {"bufferView": 0, "componentType": 5126, "count": 4, "type": "VEC3", "min": [0,0,0], "max": [1,1,0]},
            {"bufferView": 1, "componentType": 5123, "count": 6, "type": "SCALAR"}
        ],
        "bufferViews": [
            {"buffer": 0, "byteOffset": 0, "byteLength": 48},
            {"buffer": 1, "byteOffset": 0, "byteLength": 12}
        ],
        "buffers": [
            {"uri": "model_quad.bin", "byteLength": 48},
            {"uri": "model_quad_indices.bin", "byteLength": 12}
        ]
    })";
    fs.write_string("model_quad.gltf", json);

    EXPECT_NO_THROW({ Model model(fs.open("model_quad.gltf")); });

    fs.remove("model_quad.bin");
    fs.remove("model_quad_indices.bin");
    fs.remove("model_quad.gltf");
}

TEST(ModelLoad, MultiPrimitiveMeshAppliesTheSameNodeTransformToEveryPrimitive)
{
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    RenderWindow window(64, 64, "model_test_multiprim");

    HostFileSystem fs;
    std::vector<float> positionsA = TRIANGLE_POSITIONS;
    std::vector<float> positionsB = TRIANGLE_POSITIONS;
    std::vector<float> combined;
    combined.insert(combined.end(), positionsA.begin(), positionsA.end());
    combined.insert(combined.end(), positionsB.begin(), positionsB.end());
    fs.write_bytes("model_multiprim.bin", combined.data(), combined.size() * sizeof(float));

    const std::string json = R"({
        "asset": {"version": "2.0"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0, "translation": [3,4,0]}],
        "meshes": [{"primitives": [
            {"attributes": {"POSITION": 0}, "mode": 4},
            {"attributes": {"POSITION": 1}, "mode": 4}
        ]}],
        "accessors": [
            {"bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3", "min": [0,0,0], "max": [1,1,0]},
            {"bufferView": 1, "componentType": 5126, "count": 3, "type": "VEC3", "min": [0,0,0], "max": [1,1,0]}
        ],
        "bufferViews": [
            {"buffer": 0, "byteOffset": 0, "byteLength": 36},
            {"buffer": 0, "byteOffset": 36, "byteLength": 36}
        ],
        "buffers": [
            {"uri": "model_multiprim.bin", "byteLength": 72}
        ]
    })";
    fs.write_string("model_multiprim.gltf", json);

    Model model(fs.open("model_multiprim.gltf"));
    fs.remove("model_multiprim.bin");
    fs.remove("model_multiprim.gltf");

    ASSERT_EQ(ModelTestAccess::mesh_count(model), 2u);

    const Matrix4& first = ModelTestAccess::mesh_matrix(model, 0);
    const Matrix4& second = ModelTestAccess::mesh_matrix(model, 1);

    EXPECT_FLOAT_EQ(first[3].x, 3.f);
    EXPECT_FLOAT_EQ(first[3].y, 4.f);
    EXPECT_FLOAT_EQ(second[3].x, 3.f);
    EXPECT_FLOAT_EQ(second[3].y, 4.f);
}

TEST(ModelLoad, ChildNodeTransformComposesWithItsParent)
{
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    RenderWindow window(64, 64, "model_test_hierarchy");

    HostFileSystem fs;
    std::vector<float> combined;
    combined.insert(combined.end(), TRIANGLE_POSITIONS.begin(), TRIANGLE_POSITIONS.end());
    combined.insert(combined.end(), TRIANGLE_POSITIONS.begin(), TRIANGLE_POSITIONS.end());
    fs.write_bytes("model_hierarchy.bin", combined.data(), combined.size() * sizeof(float));

    const std::string json = R"({
        "asset": {"version": "2.0"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [
            {"mesh": 0, "translation": [10,0,0], "children": [1]},
            {"mesh": 1, "translation": [0,5,0]}
        ],
        "meshes": [
            {"primitives": [{"attributes": {"POSITION": 0}, "mode": 4}]},
            {"primitives": [{"attributes": {"POSITION": 1}, "mode": 4}]}
        ],
        "accessors": [
            {"bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3", "min": [0,0,0], "max": [1,1,0]},
            {"bufferView": 1, "componentType": 5126, "count": 3, "type": "VEC3", "min": [0,0,0], "max": [1,1,0]}
        ],
        "bufferViews": [
            {"buffer": 0, "byteOffset": 0, "byteLength": 36},
            {"buffer": 0, "byteOffset": 36, "byteLength": 36}
        ],
        "buffers": [
            {"uri": "model_hierarchy.bin", "byteLength": 72}
        ]
    })";
    fs.write_string("model_hierarchy.gltf", json);

    Model model(fs.open("model_hierarchy.gltf"));
    fs.remove("model_hierarchy.bin");
    fs.remove("model_hierarchy.gltf");

    ASSERT_EQ(ModelTestAccess::mesh_count(model), 2u);

    const Matrix4& rootMatrix = ModelTestAccess::mesh_matrix(model, 0);
    const Matrix4& childMatrix = ModelTestAccess::mesh_matrix(model, 1);

    EXPECT_FLOAT_EQ(rootMatrix[3].x, 10.f);
    EXPECT_FLOAT_EQ(rootMatrix[3].y, 0.f);

    // Composed: root's (10,0,0) + child's own (0,5,0), not just the child's local (0,5,0).
    EXPECT_FLOAT_EQ(childMatrix[3].x, 10.f);
    EXPECT_FLOAT_EQ(childMatrix[3].y, 5.f);
}

TEST(ModelLoad, DefaultConstructedModelHasNoHandleAndReloadMaterialsDoesNotThrow)
{
    Model model;
    EXPECT_NO_THROW(model.reload_materials());
}

class ModelTexturedTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "model_textured_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }

    // Writes a real 1x1 PNG
    static FileHandle write_textured_triangle(HostFileSystem& fs, const std::string& baseName){
        Image({1, 1}, {1.f, 1.f, 1.f, 1.f}).save(fs.open(baseName + ".png"));

        fs.write_bytes(baseName + ".bin", TRIANGLE_POSITIONS.data(), TRIANGLE_POSITIONS.size() * sizeof(float));

        const std::string json = R"({
            "asset": {"version": "2.0"},
            "scene": 0,
            "scenes": [{"nodes": [0]}],
            "nodes": [{"mesh": 0}],
            "meshes": [{"primitives": [{"attributes": {"POSITION": 0}, "material": 0, "mode": 4}]}],
            "materials": [{"pbrMetallicRoughness": {"baseColorTexture": {"index": 0}, "metallicRoughnessTexture": {"index": 0}}, "normalTexture": {"index": 0}, "occlusionTexture": {"index": 0}, "emissiveTexture": {"index": 0}}],
            "textures": [{"source": 0}],
            "images": [{"uri": ")" + baseName + R"(.png"}],
            "accessors": [
                {"bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3", "min": [0,0,0], "max": [1,1,0]}
            ],
            "bufferViews": [
                {"buffer": 0, "byteOffset": 0, "byteLength": 36}
            ],
            "buffers": [
                {"uri": ")" + baseName + R"(.bin", "byteLength": 36}
            ]
        })";
        fs.write_string(baseName + ".gltf", json);

        return fs.open(baseName + ".gltf");
    }
};

RenderWindow* ModelTexturedTest::window = nullptr;

TEST_F(ModelTexturedTest, CopyKeepsTheEmbeddedTextureAliveAfterTheSourceIsDestroyed)
{
    HostFileSystem fs;
    FileHandle handle = write_textured_triangle(fs, "model_copy_texture");

    HostFileSystem shaderFs;
    const char* vertSrc = "#version 450 core\nlayout (location = 0) in vec3 aPos;\nuniform mat4 model;\nvoid main(){ gl_Position = model * vec4(aPos, 1.0); }\n";
    const char* fragSrc = "#version 450 core\nlayout (location = 0) out vec4 outColor;\nuniform sampler2D material_baseTexture;\nvoid main(){ outColor = vec4(1.0); }\n";
    shaderFs.write_string("model_copy_v.glsl", vertSrc);
    shaderFs.write_string("model_copy_f.glsl", fragSrc);
    Shader shader(shaderFs.open("model_copy_v.glsl"), shaderFs.open("model_copy_f.glsl"));
    shaderFs.remove("model_copy_v.glsl");
    shaderFs.remove("model_copy_f.glsl");

    std::optional<Model> copy;
    {
        Model original(handle);
        fs.remove("model_copy_texture.png");
        fs.remove("model_copy_texture.bin");
        fs.remove("model_copy_texture.gltf");

        ASSERT_EQ(ModelTestAccess::embedded_texture_count(original), 5u);
        copy.emplace(original);
    } // original destroyed here

    glGetError();
    copy->render(shader, Matrix4(1.f));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ModelTexturedTest, ReloadDoesNotAccumulateDuplicateEmbeddedTextures)
{
    HostFileSystem fs;
    FileHandle handle = write_textured_triangle(fs, "model_reload_texture");

    Model model(handle);
    ASSERT_EQ(ModelTestAccess::embedded_texture_count(model), 5u);

    model.reload();
    EXPECT_EQ(ModelTestAccess::embedded_texture_count(model), 5u);

    model.reload();
    fs.remove("model_reload_texture.png");
    fs.remove("model_reload_texture.bin");
    fs.remove("model_reload_texture.gltf");

    EXPECT_EQ(ModelTestAccess::embedded_texture_count(model), 5u);
}
