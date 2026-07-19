#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/reflectable.hpp"
#include "draft/util/serialization/serializer.hpp" // IWYU pragma: keep

#include <variant>

namespace Draft {
    enum class CameraType { PERSPECTIVE, ORTHOGRAPHIC };

    struct PerspectiveCameraParams {
        DRAFT_REFLECTED(float, fov) = 45.f;
        DRAFT_REFLECTED(float, aspectRatio) = 1.f;
        DRAFT_REFLECTED(float, nearClip) = 0.1f;
        DRAFT_REFLECTED(float, farClip) = 100.f;

        DRAFT_REFLECTABLE(PerspectiveCameraParams, fov, aspectRatio, nearClip, farClip)
    };

    struct OrthographicCameraParams {
        DRAFT_REFLECTED(float, leftClip) = -1.f;
        DRAFT_REFLECTED(float, rightClip) = 1.f;
        DRAFT_REFLECTED(float, bottomClip) = -1.f;
        DRAFT_REFLECTED(float, topClip) = 1.f;
        DRAFT_REFLECTED(float, nearClip) = 0.1f;
        DRAFT_REFLECTED(float, farClip) = 100.f;
        DRAFT_REFLECTED(float, zoom) = 1.f;

        DRAFT_REFLECTABLE(OrthographicCameraParams, leftClip, rightClip, bottomClip, topClip, nearClip, farClip, zoom)
    };

    /**
     * @brief Position/orientation + view/projection matrices for a perspective or orthographic camera
     */
    class Camera {
        Vector3f position{};

        Vector3f up{ 0, 1, 0 };
        Vector3f right{ 1, 0, 0 };
        Vector3f forward{ 0, 0, -1 };
        float rotation = 0.f;

        Matrix4 projMatrix = Matrix4(1.f);
        Matrix4 viewMatrix = Matrix4(1.f);

        std::variant<PerspectiveCameraParams, OrthographicCameraParams> params;

        void update_vectors(const Vector3f& newForward);
        void update_matrices();

    public:
        Camera() = default;
        Camera(const Vector3f& position, const Vector3f& direction, PerspectiveCameraParams params);
        Camera(const Vector3f& position, const Vector3f& direction, OrthographicCameraParams params);

        // Named constructors, mirroring the old PerspectiveCamera/OrthographicCamera constructors.
        static Camera make_perspective(const Vector3f& position, const Vector3f& direction, const Vector2i& size, float fov = 45.f, float near = 0.1f, float far = 100.f);
        static Camera make_perspective(const Vector3f& position, const Vector3f& direction, float aspectRatio, float fov = 45.f, float near = 0.1f, float far = 100.f);
        static Camera make_orthographic(const Vector3f& position, const Vector3f& direction, float left, float right, float bottom, float top, float near = 0.1f, float far = 100.f);

        /**
         * @brief Points the camera to the position supplied
         * @param target
         */
        void target(const Vector3f& target);

        /**
         * @brief Points the camera to the direction supplied
         * @param dir
         */
        void point(const Vector3f& dir);

        /**
         * @brief Set the rotation of the camera
         */
        void set_rotation(float rotation);

        /**
         * @brief Only meaningful for an orthographic camera; throws std::bad_variant_access if
         * `params` currently holds PerspectiveCameraParams instead.
         */
        void set_zoom(float zoom);

        /**
         * @brief Set the camera's own position
         * @param vec
         */
        inline void set_position(const Vector3f& vec){ position = vec; point(forward); }

        inline CameraType get_type() const { return std::holds_alternative<PerspectiveCameraParams>(params) ? CameraType::PERSPECTIVE : CameraType::ORTHOGRAPHIC; }
        inline const Vector3f& get_position() const { return position; }
        inline float get_rotation() const { return rotation; }
        inline const Vector3f& get_forward() const { return forward; }
        inline const Vector3f& get_right() const { return right; }
        inline const Vector3f& get_up() const { return up; }
        inline const Matrix4& get_projection() const { return projMatrix; }
        inline const Matrix4& get_view() const { return viewMatrix; }
        inline const Matrix4 get_combined() const { return projMatrix * viewMatrix; }

        // Throw std::bad_variant_access if called on the wrong alternative.
        inline const PerspectiveCameraParams& get_perspective_params() const { return std::get<PerspectiveCameraParams>(params); }
        inline const OrthographicCameraParams& get_orthographic_params() const { return std::get<OrthographicCameraParams>(params); }

        void apply(const Shader& shader) const;
        Vector2f project(const Vector2f& point) const;
        Vector2f unproject(const Vector2f& point) const;

        // Serialization
        static void serialize(const Camera& camera, Binary::ByteArray& out);
        static void deserialize(Camera& camera, Binary::ByteView span);
        static void deserialize_and_advance(Camera& camera, Binary::ByteView& span);
        static void serialize(const Camera& camera, JSON& json);
        static void deserialize(Camera& camera, const JSON& json);
    };
}
