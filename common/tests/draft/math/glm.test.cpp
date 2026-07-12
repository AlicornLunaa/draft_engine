#include <gtest/gtest.h>
#include "draft/math/glm.hpp"

using namespace Draft;

TEST(Glm, VectorAliasesConstructAndAccess)
{
    Vector2f v2(1.f, 2.f);
    EXPECT_FLOAT_EQ(v2.x, 1.f);
    EXPECT_FLOAT_EQ(v2.y, 2.f);

    Vector3f v3(1.f, 2.f, 3.f);
    EXPECT_FLOAT_EQ(v3.z, 3.f);

    Vector4f v4(1.f, 2.f, 3.f, 4.f);
    EXPECT_FLOAT_EQ(v4.w, 4.f);
}

TEST(Glm, MatrixAliasIsGlmMat)
{
    Matrix4 m(1.f);
    EXPECT_FLOAT_EQ(m[0][0], 1.f);
    EXPECT_FLOAT_EQ(m[1][1], 1.f);
    EXPECT_FLOAT_EQ(m[0][1], 0.f);
}

TEST(Glm, QuaternionIsARealQuaternion)
{
    // Regression guard
    Quaternion q = Math::identity<Quaternion>();
    EXPECT_FLOAT_EQ(q.w, 1.f);
    EXPECT_FLOAT_EQ(q.x, 0.f);
}

TEST(Glm, FastModelMatrixTranslationOnly)
{
    Matrix4 m = Optimal::fast_model_matrix(Vector2f(5.f, 10.f), 0.f, Vector2f(1.f, 1.f), Vector2f(0.f, 0.f), 2);
    Matrix4 expected = Math::translate(Matrix4(1.f), Vector3f(5.f, 10.f, 2.f));

    for(int col = 0; col < 4; col++){
        for(int row = 0; row < 4; row++){
            EXPECT_FLOAT_EQ(m[col][row], expected[col][row]) << "at [" << col << "][" << row << "]";
        }
    }
}

TEST(Glm, FastModelMatrixRotationOnly)
{
    float angle = Math::half_pi<float>();
    Matrix4 m = Optimal::fast_model_matrix(Vector2f(0.f, 0.f), angle, Vector2f(1.f, 1.f), Vector2f(0.f, 0.f), 0);
    Matrix4 expected = Math::rotate(Matrix4(1.f), angle, Vector3f(0.f, 0.f, 1.f));

    for(int col = 0; col < 4; col++){
        for(int row = 0; row < 4; row++){
            EXPECT_NEAR(m[col][row], expected[col][row], 1e-6f) << "at [" << col << "][" << row << "]";
        }
    }
}

TEST(Glm, FastModelMatrixScaleOnly)
{
    Matrix4 m = Optimal::fast_model_matrix(Vector2f(0.f, 0.f), 0.f, Vector2f(2.f, 3.f), Vector2f(0.f, 0.f), 0);

    EXPECT_FLOAT_EQ(m[0][0], 2.f);
    EXPECT_FLOAT_EQ(m[1][1], 3.f);
}

TEST(Glm, FastModelMatrixZIndexSetsZTranslation)
{
    Matrix4 m = Optimal::fast_model_matrix(Vector2f(0.f, 0.f), 0.f, Vector2f(1.f, 1.f), Vector2f(0.f, 0.f), 7);
    EXPECT_FLOAT_EQ(m[3][2], 7.f);
}
