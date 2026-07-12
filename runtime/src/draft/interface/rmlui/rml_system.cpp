#include "draft/interface/rmlui/rml_system.hpp"
#include "draft/interface/rmlui/RmlUi_Platform_GLFW.h"
#include "draft/interface/rmlui/RmlUi_Renderer_GL3.h"
#include "draft/interface/rmlui/rml_context.hpp"
#include "draft/rendering/render_window.hpp"

#include "RmlUi/Core/Core.h"

#include <algorithm>
#include <cassert>

namespace Draft {
    // Statics
    int RmlUiSystem::s_backendCount = 0;
    SystemInterface_GLFW* RmlUiSystem::s_systemInterface = nullptr;
    RenderInterface_GL3* RmlUiSystem::s_renderInterface = nullptr;

    // Constructors
    RmlUiSystem::RmlUiSystem(RenderWindow& window){
        // Initialize RML if this is the first object
        if(s_backendCount <= 0){
            s_renderInterface = new RenderInterface_GL3();
            s_systemInterface = new SystemInterface_GLFW();

            Rml::SetRenderInterface(s_renderInterface);
            Rml::SetSystemInterface(s_systemInterface);
            Rml::Initialise();
        }

        // Initialize interfaces
        auto size = window.get_frame_size();
        s_systemInterface->SetWindow(window.get_glfw_handle());
        s_renderInterface->SetViewport(size.x, size.y);

        s_backendCount++;
    }

    RmlUiSystem::~RmlUiSystem(){
        s_backendCount--;

        if(s_backendCount <= 0){
            Rml::Shutdown();

            delete s_renderInterface;
            delete s_systemInterface;
            s_renderInterface = nullptr;
            s_systemInterface = nullptr;
        }
    }

    // Private functions
    void RmlUiSystem::register_context(RmlContext& context){
        m_contexts.push_back(&context);
    }

    void RmlUiSystem::unregister_context(RmlContext& context){
        std::erase(m_contexts, &context);
    }

    // Functions
    void RmlUiSystem::render(Time, RenderLayer){
        for(RmlContext* context : m_contexts)
            context->render();
    }

    bool RmlUiSystem::on_event(const Event& event){
        for(RmlContext* context : m_contexts){
            if(context->handle_event(event))
                return true;
        }

        return false;
    }
}
