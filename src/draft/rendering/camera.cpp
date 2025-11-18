#include "draft/rendering/camera.hpp"
#include "draft/math/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/matrix.hpp"

#include "glad/gl.h"

namespace Draft {
    // Abstract camera
    // Private functions
    void Camera::update_vectors(const Vector3f& newForward){
        forward = newForward;
        right = Math::normalize(Math::cross(up, forward));
    }

    // Constructor
    Camera::Camera(const Vector3f& position, const Vector3f& direction) : position(position), forward(direction) {
        update_vectors(forward);
        point(direction);
    }

    // Functions
    void Camera::target(const Vector3f& target){
        // Sets the view matrix to a target
        update_vectors(Math::normalize(target - position));
        viewMatrix = Math::lookAt(position, position + forward, up);
    }

    void Camera::point(const Vector3f& dir){
        // Sets the view matrix to a direction
        target(position + dir);
    }

    void Camera::set_rotation(float rotation){
        // Set the rotation
        up = { -Math::sin(rotation), Math::cos(rotation), 0.0f };
        update_vectors(forward);
        viewMatrix = Math::lookAt(position, position + forward, up);
        this->rotation = rotation;
    }

    void Camera::apply(const Shader& shader) const {
        shader.set_uniform("view", get_view());
        shader.set_uniform("projection", get_projection());
    }

    Vector2f Camera::project(const Vector2f& point) const {
        return projMatrix * viewMatrix * Vector4f(point.x, point.y, 0, 1);
    }

    Vector2f Camera::unproject(const Vector2f& point) const {
        return Math::inverse(projMatrix * viewMatrix) * Vector4f(point.x, point.y, 0, 1);
    }

    // Perspective camera
    void PerspectiveCamera::update_matrices(){ projMatrix = Math::perspective(fov, aspectRatio, nearClip, farClip); }

    PerspectiveCamera::PerspectiveCamera(const Vector3f& position, const Vector3f& direction, const Vector2i& size, float fov, float nearClip, float farClip)
        : Camera(position, direction), fov(fov), aspectRatio((float)size.x / size.y), nearClip(nearClip), farClip(farClip) {
        update_matrices();
    }

    // Orthographic camera
    void OrthographicCamera::update_matrices(){ projMatrix = Math::ortho(leftClip * zoom, rightClip * zoom, bottomClip * zoom, topClip * zoom, nearClip, farClip); }
    
    OrthographicCamera::OrthographicCamera(const Vector3f& position, const Vector3f& direction, float left, float right, float bottom, float top, float nearClip, float farClip)
        : Camera(position, direction), leftClip(left), rightClip(right), bottomClip(bottom), topClip(top), nearClip(nearClip), farClip(farClip) {
        update_matrices();
    }

    void OrthographicCamera::set_zoom(float z){
        zoom = z;
        update_matrices();
    }
};