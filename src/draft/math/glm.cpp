#include "draft/math/glm.hpp"
#include "glm/trigonometric.hpp"

namespace Draft::Optimal {
    Matrix4 fast_model_matrix(const Vector2f& translation, float rotation, const Vector2f& scale, const Vector2f& origin, int zIndex){
        // Magic construction for a 2d model matrix to avoid costly matrix math :)
        float cosTheta = Math::cos(rotation);
        float sinTheta = Math::sin(rotation);

        Matrix4 matrix(1.f);

        matrix[0][0] = scale.x * cosTheta;
        matrix[1][0] = -scale.y * sinTheta;
        matrix[3][0] = translation.x + origin.x * (1 - cosTheta) + origin.y * sinTheta - origin.x;

        matrix[0][1] = scale.x * sinTheta;
        matrix[1][1] = scale.y * cosTheta;
        matrix[3][1] = translation.y + origin.y * (1 - cosTheta) - origin.x * sinTheta - origin.y;

        matrix[3][2] = zIndex;

        return matrix;
    }
}