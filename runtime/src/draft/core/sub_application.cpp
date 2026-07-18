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
    }
}
