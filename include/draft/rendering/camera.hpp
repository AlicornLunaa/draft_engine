#pragma once

#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/shader.hpp"

namespace Draft {
    class Camera {
    protected:
        // Variables
        Rect<float> viewport = { 0, 0, -1, -1 };
        Vector3f position{};

        Vector3f up{ 0, 1, 0 };
        Vector3f right{ 1, 0, 0 };
        Vector3f forward{ 0, 0, -1 };

        Matrix4 projMatrix = Matrix4(1.f);
        Matrix4 viewMatrix = Matrix4(1.f);

        // Private functions
        void update_vectors(const Vector3f& newForward);
        virtual void update_matrices() = 0;

    public:
        // Constructor
        Camera(const Vector3f& position, const Vector3f& direction);

        // Functions
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
         * @brief Set the camera's own position
         * @param vec 
         */
        inline void set_position(const Vector3f& vec){ position = vec; point(forward); }

        /**
         * @brief Set the viewport object
         * @param rect 
         */
        inline void set_viewport(const Rect<float>& rect){ viewport = rect; }

        inline const Vector3f& get_position() const { return position; }
        inline const FloatRect& get_viewport() const { return viewport; }
        inline const Vector3f& get_forward() const { return forward; }
        inline const Vector3f& get_right() const { return right; }
        inline const Vector3f& get_up() const { return up; }
        inline const Matrix4& get_projection() const { return projMatrix; }
        inline const Matrix4& get_view() const { return viewMatrix; }

        void apply(const RenderWindow& window, const std::shared_ptr<Shader>& shader) const;
        Vector2f project(const Vector2f& point) const;
        Vector2f unproject(const Vector2f& point) const;
    };

    class PerspectiveCamera : public Camera {
    protected:
        // Variables
        float fov;
        float aspectRatio;
        float nearClip;
        float farClip;

        // Private functions
        virtual void update_matrices();

    public:
        // Constructor
        PerspectiveCamera(const Vector3f& position, const Vector3f& direction, const Vector2i& size, float fov = 45.f, float near = 0.1f, float far = 100.f);
    };

    class OrthographicCamera : public Camera {
    protected:
        // Variables
        float leftClip;
        float rightClip;
        float bottomClip;
        float topClip;
        float nearClip;
        float farClip;
        float zoom = 1.f;

        // Private functions
        virtual void update_matrices();

    public:
        // Constructor
        OrthographicCamera(const Vector3f& position, const Vector3f& direction, float left, float right, float bottom, float top, float near = 0.1f, float far = 100.f);

        // Functions
        inline float get_left() const { return leftClip; }
        inline float get_right() const { return rightClip; }
        inline float get_bottom() const { return bottomClip; }
        inline float get_top() const { return topClip; }
        inline float get_near() const { return nearClip; }
        inline float get_far() const { return farClip; }
        void set_zoom(float z);
        float get_zoom();
    };
};