#include "draft/rendering/camera.hpp"
#include "draft/math/glm.hpp"
#include "glm/matrix.hpp"

#include "glad/gl.h"

namespace Draft {
    // Abstract camera
    // Private functions
    void Camera::update_vectors(){
        right = Math::normalize(Math::cross({0, 1, 0}, forward));
        up = Math::cross(forward, right);
    }

    // Constructor
    Camera::Camera(const Vector3f& position, const Vector3f& direction) : position(position), forward(direction) {
        update_vectors();
        point(direction);
    }

    // Functions
    void Camera::target(const Vector3f& target){
        // Sets the view matrix to a target
        forward = Math::normalize(target - position);
        update_vectors();

        viewMatrix = Matrix4(1.f);
        viewMatrix[0][0] = right.x; viewMatrix[0][1] = right.y; viewMatrix[0][2] = right.z;
        viewMatrix[1][0] = up.x; viewMatrix[1][1] = up.y; viewMatrix[1][2] = up.z;
        viewMatrix[2][0] = forward.x; viewMatrix[2][1] = forward.y; viewMatrix[2][2] = forward.z;
        viewMatrix = Math::translate(viewMatrix, position);
    }

    void Camera::point(const Vector3f& dir){
        // Sets the view matrix to a direction
        target(position + forward);
    }

    void Camera::apply(const RenderWindow& window, Shader& shader) const {
        if(viewport.width <= 0 || viewport.height <= 0){
            auto size = window.get_size();
            glViewport(0, 0, size.x, size.y);
        } else {
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
        }

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

    float OrthographicCamera::get_zoom(){ return zoom; }
};