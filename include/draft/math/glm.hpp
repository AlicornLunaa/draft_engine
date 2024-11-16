#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp" // IWYU pragma: keep, Includes definitions and such
#include "glm/ext/matrix_transform.hpp" // IWYU pragma: keep, Includes definitions and such
#include "glm/gtx/matrix_transform_2d.hpp" // IWYU pragma: keep, Includes definitions and such
#include "glm/ext/matrix_clip_space.hpp" // IWYU pragma: keep, Includes definitions and such
#include "glm/gtx/rotate_vector.hpp" // IWYU pragma: keep, Includes definitions and such
#include "glm/geometric.hpp" // IWYU pragma: keep, Includes definitions and such

namespace Draft {
    // Aliases for GLM
    namespace Math = glm;

    // Vectors
    template<size_t L, typename T, glm::qualifier Q = glm::defaultp> using Vector = Math::vec<L, T, Q>;
    template<typename T, glm::qualifier Q = glm::defaultp> using Vector2 = Math::vec<2, T, Q>;
    template<typename T, glm::qualifier Q = glm::defaultp> using Vector3 = Math::vec<3, T, Q>;
    template<typename T, glm::qualifier Q = glm::defaultp> using Vector4 = Math::vec<4, T, Q>;

    typedef Vector2<float> Vector2f;
    typedef Vector2<double> Vector2d;
    typedef Vector2<int> Vector2i;
    typedef Vector2<unsigned int> Vector2u;
    typedef Vector3<float> Vector3f;
    typedef Vector3<double> Vector3d;
    typedef Vector3<int> Vector3i;
    typedef Vector3<unsigned int> Vector3u;
    typedef Vector4<float> Vector4f;
    typedef Vector4<double> Vector4d;
    typedef Vector4<int> Vector4i;
    typedef Vector4<unsigned int> Vector4u;

    // Matrix
    template<typename T, size_t rows, size_t cols>
    using Matrix = Math::mat<cols, rows, T>;

    typedef Matrix<float, 2, 2> Matrix2;
    typedef Matrix<float, 3, 3> Matrix3;
    typedef Matrix<float, 4, 4> Matrix4;
    typedef Matrix<float, 4, 4> Quaternion;

    namespace Optimal {
        Matrix4 fast_model_matrix(const Vector2f& translation, float rotation, const Vector2f& scale, const Vector2f& origin, int zIndex);
    }
}