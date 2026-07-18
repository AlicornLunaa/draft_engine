#define GLFW_INCLUDE_NONE

#include "draft/core/sub_application.hpp"

#include "GLFW/glfw3.h"

namespace {
    Draft::GLFWProperties hidden_window_properties(){
        Draft::GLFWProperties props = Draft::Window::get_default_properties();
        props.push_back({GLFW_VISIBLE, GLFW_FALSE});
        return props;
    }
}

namespace Draft {
    SubApplication::SubApplication(RenderWindow& sharedContext, const Vector2u& renderSize)
        : ApplicationInterface(m_window, m_keyboard, m_mouse),
          m_sharedContext(sharedContext),
          m_window(renderSize.x, renderSize.y, "", hidden_window_properties(), sharedContext.get_glfw_handle()),
          m_keyboard(m_window),
          m_mouse(m_window),
          m_target({renderSize})
    {
        p_renderer = std::make_unique<DefaultRenderer>(renderSize);
        glfwMakeContextCurrent(m_sharedContext.get_glfw_handle());
    }

    void SubApplication::step(Time dt){
        glfwMakeContextCurrent(m_window.get_glfw_handle());

        deltaTime = dt;

        if(p_newRenderer){
            p_renderer = std::move(p_newRenderer);
            p_newRenderer = nullptr;
        }

        if(p_newScene)
            scene_change();

        tick();
        frame_into(m_target);

        glfwMakeContextCurrent(m_sharedContext.get_glfw_handle());
    }

    void SubApplication::resize(const Vector2u& size){
        m_target.resize(size);

        if(p_renderer)
            p_renderer->resize(size);
    }
}
