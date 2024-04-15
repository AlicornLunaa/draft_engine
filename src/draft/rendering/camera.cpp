#include "draft/rendering/camera.hpp"
#include "draft/math/matrix.hpp"
#include "draft/math/vector3.hpp"

namespace Draft {
    // Abstract camera
    // Private functions
    void Camera::update_vectors(){
        right = Vector3f(0, 1, 0).cross(forward).normalized();
        up = forward.cross(right);
    }

    // Constructor
    Camera::Camera(const Vector3f& position, const Vector3f& direction) : position(position), forward(direction) {
        update_vectors();
        point(direction);
    }

    // Functions
    void Camera::target(const Vector3f& target){
        // Sets the view matrix to a target
        forward = (target - position).normalized();
        update_vectors();

        viewMatrix = Matrix4::identity();
        viewMatrix[0][0] = right.x; viewMatrix[0][1] = right.y; viewMatrix[0][2] = right.z;
        viewMatrix[1][0] = up.x; viewMatrix[1][1] = up.y; viewMatrix[1][2] = up.z;
        viewMatrix[2][0] = forward.x; viewMatrix[2][1] = forward.y; viewMatrix[2][2] = forward.z;
        viewMatrix *= Matrix4::translation(position);
    }

    void Camera::point(const Vector3f& dir){
        // Sets the view matrix to a direction
        target(position + forward);
    }

    void Camera::apply(Shader& shader) const {
        shader.set_uniform("view", get_view());
        shader.set_uniform("projection", get_projection());
    }

    // Perspective camera
    void PerspectiveCamera::update_matrices(){ projMatrix = Matrix4::perspective(fov, aspectRatio, nearClip, farClip); }

    PerspectiveCamera::PerspectiveCamera(const Vector3f& position, const Vector3f& direction, const Vector2i& size, float fov, float near, float far)
        : Camera(position, direction), fov(fov), aspectRatio((float)size.x / size.y), nearClip(near), farClip(far) {
        update_matrices();
    }

    // Orthographic camera
    void OrthographicCamera::update_matrices(){ projMatrix = Matrix4::orthographic(leftClip, rightClip, bottomClip, topClip, nearClip, farClip); }
    
    OrthographicCamera::OrthographicCamera(const Vector3f& position, const Vector3f& direction, float left, float right, float bottom, float top, float near, float far)
        : Camera(position, direction), leftClip(left), rightClip(right), bottomClip(bottom), topClip(top), nearClip(near), farClip(far) {
        update_matrices();
    }
};