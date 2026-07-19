#include "draft/editor/freecam_controller.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/rendering/camera.hpp"

namespace Draft {
    namespace {
        constexpr float PAN_SPEED = 400.f; // world units/sec at zoom 1
        constexpr float ZOOM_STEP = 1.1f;
        constexpr float MIN_ZOOM = 0.01f;
    }

    FreecamControllerSystem::FreecamControllerSystem(EditorApplication& app) : m_app(app) {}

    void FreecamControllerSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        if(!m_app.gameApp.simulationPaused){
            // Playing, hand control back to whatever gameScene's own CameraComponent resolves to.
            m_app.gameScene.set_active_camera_override(std::nullopt);
            return;
        }

        if(m_app.viewportFocused){
            Keyboard& keyboard = m_app.application.keyboard;
            float distance = PAN_SPEED * m_zoom * dt.as_seconds();

            if(keyboard.is_pressed(Keyboard::W) || keyboard.is_pressed(Keyboard::UP))
                m_position.y -= distance;
            if(keyboard.is_pressed(Keyboard::S) || keyboard.is_pressed(Keyboard::DOWN))
                m_position.y += distance;
            if(keyboard.is_pressed(Keyboard::A) || keyboard.is_pressed(Keyboard::LEFT))
                m_position.x -= distance;
            if(keyboard.is_pressed(Keyboard::D) || keyboard.is_pressed(Keyboard::RIGHT))
                m_position.x += distance;
        }

        if(m_app.viewportHovered){
            double scrollY = m_app.application.mouse.get_scroll().y;

            if(scrollY != m_lastScrollY){
                m_zoom *= scrollY > 0 ? 1.f / ZOOM_STEP : ZOOM_STEP;
                m_zoom = Math::max(m_zoom, MIN_ZOOM);
                m_lastScrollY = scrollY;
            }
        }
        
        Vector2u size = Math::max(m_app.gameApp.get_output().get_properties().size, Vector2u(1, 1));
        float aspect = (float)size.x / (float)size.y;
        float halfHeight = size.y * m_zoom;
        float halfWidth = halfHeight * aspect;

        Camera camera = Camera::make_orthographic(
            {m_position.x, m_position.y, 10.f}, {0.f, 0.f, -1.f},
            -halfWidth, halfWidth, halfHeight, -halfHeight
        );

        m_app.gameScene.set_active_camera_override(std::move(camera));
    }
}
