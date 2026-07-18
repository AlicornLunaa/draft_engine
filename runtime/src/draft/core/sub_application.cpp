#include "draft/core/sub_application.hpp"

namespace Draft {
    SubApplication::SubApplication(const Vector2u& size)
        : ApplicationInterface(m_target, fakeKeyboard, fakeMouse), fakeMouse(size), m_target({.size = size})
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
        fakeMouse.set_window_size(size);

        if(p_renderer)
            p_renderer->resize(size);

        // Mirrors Application::trigger_resize()
        Event event;
        event.type = Event::Resized;
        event.size.width = size.x;
        event.size.height = size.y;
        dispatch(event);
    }

    bool SubApplication::inject_event(const Event& event){
        return dispatch(event);
    }
}
