#include "draft/core/sub_application.hpp"

namespace Draft {
    SubApplication::SubApplication(const Vector2u& size, Keyboard& keyboard, Mouse& mouse)
        : ApplicationInterface(m_target, keyboard, mouse), m_target({.size = size})
    {
        p_renderer = std::make_unique<DefaultRenderer>(target.get_size());
    }

    void SubApplication::step(Time dt){
        deltaTime = dt;

        if(p_newRenderer){
            p_renderer = std::move(p_newRenderer);
            p_newRenderer = nullptr;
        }

        if(p_newScene)
            scene_change();

        tick();
        frame_into(target);
    }

    void SubApplication::resize(const Vector2u& size){
        target.set_size(size);

        if(p_renderer)
            p_renderer->resize(size);

        // Mirrors Application::trigger_resize(): a nested ImGuiSystem/RmlUiSystem has no real
        // window to read a live size from every frame (that's what the old GLFW-backed versions
        // did for free), so it only learns about a resize by way of this event reaching its
        // on_event() - nothing else calls it since target/p_renderer above cover the Framebuffer/
        // Renderer side of a resize but not the active scene's own systems.
        Event event;
        event.type = Event::Resized;
        event.size.width = size.x;
        event.size.height = size.y;
        dispatch(event);
    }
}
