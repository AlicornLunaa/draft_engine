#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/mesh.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

TEST(Mesh, DefaultConstructedMeshHasNoFlagsSet)
{
    Mesh mesh;
    EXPECT_FALSE(mesh.is_indexed());
    EXPECT_FALSE(mesh.is_uv_mapped());
    EXPECT_FALSE(mesh.is_color_mapped());
    EXPECT_TRUE(mesh.get_vertices().empty());
}

TEST(Mesh, RawConstructorStoresVerticesWithNoOtherFlags)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    Mesh mesh(verts);

    EXPECT_FALSE(mesh.is_indexed());
    EXPECT_FALSE(mesh.is_uv_mapped());
    EXPECT_FALSE(mesh.is_color_mapped());
    EXPECT_EQ(mesh.get_vertices().size(), 3u);
    EXPECT_FLOAT_EQ(mesh.get_vertices()[1].x, 1.f);
}

TEST(Mesh, IndexedConstructorSetsIsIndexed)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    std::vector<int> indices{0, 1, 2};
    Mesh mesh(verts, indices);

    EXPECT_TRUE(mesh.is_indexed());
    EXPECT_FALSE(mesh.is_uv_mapped());
    EXPECT_EQ(mesh.get_indices().size(), 3u);
    EXPECT_EQ(mesh.get_indices()[2], 2);
}

TEST(Mesh, UvMappedConstructorSetsIsUvMapped)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}};
    std::vector<Vector2f> uvs{{0, 0}, {1, 0}};
    Mesh mesh(verts, uvs);

    EXPECT_TRUE(mesh.is_uv_mapped());
    EXPECT_FALSE(mesh.is_indexed());
    EXPECT_FLOAT_EQ(mesh.get_tex_coords()[1].x, 1.f);
}

TEST(Mesh, ColorMappedConstructorSetsIsColorMapped)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}};
    std::vector<Vector3f> colors{{1, 0, 0}, {0, 1, 0}};
    Mesh mesh(verts, colors);

    EXPECT_TRUE(mesh.is_color_mapped());
    EXPECT_FALSE(mesh.is_indexed());
    EXPECT_FLOAT_EQ(mesh.get_colors()[1].y, 1.f);
}

TEST(Mesh, FullyIndexedUvColorMappedConstructorSetsAllThreeFlags)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    std::vector<int> indices{0, 1, 2};
    std::vector<Vector2f> uvs{{0, 0}, {1, 0}, {0, 1}};
    std::vector<Vector3f> colors{{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    Mesh mesh(verts, indices, uvs, colors);

    EXPECT_TRUE(mesh.is_indexed());
    EXPECT_TRUE(mesh.is_uv_mapped());
    EXPECT_TRUE(mesh.is_color_mapped());
    EXPECT_EQ(mesh.get_indices().size(), 3u);
    EXPECT_EQ(mesh.get_tex_coords().size(), 3u);
    EXPECT_EQ(mesh.get_colors().size(), 3u);
}

// DrawableMesh issues real GL calls (builds a VertexArray), so its tests share one hidden
// RenderWindow/GL context instead of creating one per test - same pattern as texture.test.cpp.
class DrawableMeshTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "drawable_mesh_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* DrawableMeshTest::window = nullptr;

TEST_F(DrawableMeshTest, ConstructionAllocatesARealVertexArray)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    DrawableMesh mesh{Mesh(verts)};

    mesh.get_vertex_array().bind();
    GLint boundVao = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundVao);
    EXPECT_NE(boundVao, 0);
    mesh.get_vertex_array().unbind();
}

TEST_F(DrawableMeshTest, RenderNonIndexedMeshProducesNoGLError)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    DrawableMesh mesh{Mesh(verts)};

    glGetError(); // Clear any pending error before the assertion below
    mesh.render();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(DrawableMeshTest, RenderIndexedMeshProducesNoGLError)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    std::vector<int> indices{0, 1, 2};
    DrawableMesh mesh{Mesh(verts, indices)};

    glGetError();
    mesh.render();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(DrawableMeshTest, CopyConstructorProducesAnIndependentlyRenderableVertexArray)
{
    std::vector<Vector3f> verts{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    DrawableMesh original{Mesh(verts)};
    DrawableMesh copy(original);

    EXPECT_NE(&original.get_vertex_array(), &copy.get_vertex_array());

    glGetError();
    copy.render();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
