#include "draft/interface/rmlui/rml_engine.hpp"
#include "draft/interface/rmlui/RmlUi_Platform_GLFW.h"
#include "draft/interface/rmlui/RmlUi_Renderer_GL3.h"
#include "draft/rendering/render_window.hpp"

#include "RmlUi/Core/Core.h"

#include <cstdlib>

namespace Draft {
    // Statics
    int RmlEngine::s_backendCount = 0;
    SystemInterface_GLFW* RmlEngine::s_systemInterface = nullptr;
    RenderInterface_GL3* RmlEngine::s_renderInterface = nullptr;

    // Constructors
    RmlEngine::RmlEngine(RenderWindow& window) : m_windowRef(window) {
        // Initialize RML if this is the first object
        if(s_backendCount <= 0){
            s_renderInterface = new RenderInterface_GL3();
            s_systemInterface = new SystemInterface_GLFW();

            Rml::SetRenderInterface(s_renderInterface);
            Rml::SetSystemInterface(s_systemInterface);
            Rml::Initialise();
        }

        // Initialize interfaces
        auto size = window.get_size();
        s_systemInterface->SetWindow(window.get_glfw_handle());
        s_renderInterface->SetViewport(size.x, size.y);

        s_backendCount++;
    }

    RmlEngine::RmlEngine(RmlEngine&& other) : m_windowRef(other.m_windowRef) {
    }

    RmlEngine::~RmlEngine(){
        s_backendCount--;

        if(s_backendCount <= 0){
            Rml::Shutdown();

            delete s_renderInterface;
            delete s_systemInterface;
        }
    }

    // Functions
    void RmlEngine::start_frame() const {
        s_renderInterface->BeginFrame();
    }

    void RmlEngine::end_frame() const {
        s_renderInterface->EndFrame();
    }
}