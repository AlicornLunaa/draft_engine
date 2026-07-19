#include "draft/rendering/camera.hpp"
#include "draft/math/glm.hpp"

#include <type_traits>

namespace Draft {
    // Private functions
    void Camera::update_vectors(const Vector3f& newForward){
        forward = newForward;
        right = Math::normalize(Math::cross(up, forward));
    }

    void Camera::update_matrices(){
        std::visit([this](auto&& p){
            using P = std::decay_t<decltype(p)>;

            if constexpr (std::is_same_v<P, PerspectiveCameraParams>){
                projMatrix = Math::perspective(p.fov, p.aspectRatio, p.nearClip, p.farClip);
            } else {
                projMatrix = Math::ortho(p.leftClip * p.zoom, p.rightClip * p.zoom, p.bottomClip * p.zoom, p.topClip * p.zoom, p.nearClip, p.farClip);
            }
        }, params);
    }

    // Constructors
    Camera::Camera(const Vector3f& position, const Vector3f& direction, PerspectiveCameraParams p)
        : position(position), forward(direction), params(p) {
        update_vectors(forward);
        point(direction);
        update_matrices();
    }

    Camera::Camera(const Vector3f& position, const Vector3f& direction, OrthographicCameraParams p)
        : position(position), forward(direction), params(p) {
        update_vectors(forward);
        point(direction);
        update_matrices();
    }

    Camera Camera::make_perspective(const Vector3f& position, const Vector3f& direction, const Vector2i& size, float fov, float nearClip, float farClip){
        return make_perspective(position, direction, (float)size.x / size.y, fov, nearClip, farClip);
    }

    Camera Camera::make_perspective(const Vector3f& position, const Vector3f& direction, float aspectRatio, float fov, float nearClip, float farClip){
        return Camera(position, direction, PerspectiveCameraParams{fov, aspectRatio, nearClip, farClip});
    }

    Camera Camera::make_orthographic(const Vector3f& position, const Vector3f& direction, float left, float right, float bottom, float top, float nearClip, float farClip){
        return Camera(position, direction, OrthographicCameraParams{left, right, bottom, top, nearClip, farClip, 1.f});
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

    void Camera::set_zoom(float zoom){
        std::get<OrthographicCameraParams>(params).zoom = zoom;
        update_matrices();
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

    // Serialization: position/direction/rotation/params are the only real state, up/right/proj/view
    // are all derived and get recomputed by replaying construction + set_rotation() below.
    void Camera::serialize(const Camera& camera, Binary::ByteArray& out){
        Serializer::serialize(camera.position, out);
        Serializer::serialize(camera.forward, out);
        Serializer::serialize(camera.rotation, out);
        Serializer::serialize(camera.params, out);
    }

    void Camera::deserialize(Camera& camera, Binary::ByteView span){
        deserialize_and_advance(camera, span);
    }

    void Camera::deserialize_and_advance(Camera& camera, Binary::ByteView& span){
        Vector3f position{};
        Vector3f direction{};
        float rotation = 0.f;
        std::variant<PerspectiveCameraParams, OrthographicCameraParams> params;

        Serializer::deserialize_and_advance(position, span);
        Serializer::deserialize_and_advance(direction, span);
        Serializer::deserialize_and_advance(rotation, span);
        Serializer::deserialize_and_advance(params, span);

        camera = std::visit([&](auto&& p){ return Camera(position, direction, p); }, params);
        camera.set_rotation(rotation);
    }

    void Camera::serialize(const Camera& camera, JSON& json){
        Serializer::serialize(camera.position, json["position"]);
        Serializer::serialize(camera.forward, json["direction"]);
        Serializer::serialize(camera.rotation, json["rotation"]);
        Serializer::serialize(camera.params, json["params"]);
    }

    void Camera::deserialize(Camera& camera, const JSON& json){
        Vector3f position{};
        Vector3f direction{};
        float rotation = 0.f;
        std::variant<PerspectiveCameraParams, OrthographicCameraParams> params;

        Serializer::deserialize(position, json.at("position"));
        Serializer::deserialize(direction, json.at("direction"));
        Serializer::deserialize(rotation, json.at("rotation"));
        Serializer::deserialize(params, json.at("params"));

        camera = std::visit([&](auto&& p){ return Camera(position, direction, p); }, params);
        camera.set_rotation(rotation);
    }
}
