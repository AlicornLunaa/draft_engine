#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/batching/collection.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

using namespace Draft;

namespace {
    // Collection::flush() is pure virtual. A trivial concrete subclass is needed to construct
    // one at all, and to read the protected debug textures/dirty flag for verification.
    class TestCollection : public Collection {
    public:
        void flush() override {}

        using Collection::whiteTexture;
        using Collection::blackTexture;
        using Collection::normalTexture;
        using Collection::p_matricesDirty;
    };
}

// Collection's constructor allocates 3 real debug textures, so the whole suite shares one hidden
// RenderWindow/GL context instead of creating one per test.
class CollectionTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "collection_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* CollectionTest::window = nullptr;

TEST_F(CollectionTest, ConstructionAllocatesThreeRealDebugTextures)
{
    TestCollection collection;

    EXPECT_NE(collection.whiteTexture.get_texture_handle(), 0u);
    EXPECT_NE(collection.blackTexture.get_texture_handle(), 0u);
    EXPECT_NE(collection.normalTexture.get_texture_handle(), 0u);

    // All three must be distinct GL objects, not accidentally the same handle
    EXPECT_NE(collection.whiteTexture.get_texture_handle(), collection.blackTexture.get_texture_handle());
    EXPECT_NE(collection.whiteTexture.get_texture_handle(), collection.normalTexture.get_texture_handle());
}

TEST_F(CollectionTest, DefaultMatricesProduceAnIdentityCombinedMatrix)
{
    TestCollection collection;
    EXPECT_EQ(collection.get_combined_matrix(), Matrix4(1.f));
}

TEST_F(CollectionTest, SetProjMatrixUpdatesCombinedMatrixAndDirtiesIt)
{
    TestCollection collection;
    collection.p_matricesDirty = false;

    Matrix4 proj = glm::ortho(0.f, 100.f, 0.f, 100.f);
    collection.set_proj_matrix(proj);

    EXPECT_EQ(collection.get_proj_matrix(), proj);
    EXPECT_EQ(collection.get_combined_matrix(), proj * collection.get_trans_matrix());
    EXPECT_TRUE(collection.p_matricesDirty);
}

TEST_F(CollectionTest, SetTransMatrixUpdatesCombinedMatrixAndDirtiesIt)
{
    TestCollection collection;
    collection.p_matricesDirty = false;

    Matrix4 trans = glm::translate(Matrix4(1.f), Vector3f(1.f, 2.f, 3.f));
    collection.set_trans_matrix(trans);

    EXPECT_EQ(collection.get_trans_matrix(), trans);
    EXPECT_EQ(collection.get_combined_matrix(), collection.get_proj_matrix() * trans);
    EXPECT_TRUE(collection.p_matricesDirty);
}
