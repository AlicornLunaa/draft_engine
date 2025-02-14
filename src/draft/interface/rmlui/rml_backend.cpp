#include "draft/interface/rmlui/rml_backend.hpp"
#include "draft/interface/rmlui/RmlUi_Platform_GLFW.h"
#include "draft/interface/rmlui/RmlUi_Renderer_GL3.h"
#include "draft/rendering/render_window.hpp"
#include "RmlUi/Core/Core.h"
#include <cstdlib>

namespace Draft::UI {
    // Statics
    int RMLBackend::s_backendCount = 0;
    SystemInterface_GLFW* RMLBackend::s_systemInterface = nullptr;
    RenderInterface_GL3* RMLBackend::s_renderInterface = nullptr;

    // Constructors
    RMLBackend::RMLBackend(RenderWindow& window) : m_windowRef(window) {
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

    RMLBackend::~RMLBackend(){
        s_backendCount--;

        if(s_backendCount <= 0){
            Rml::Shutdown();

            delete s_renderInterface;
            delete s_systemInterface;
        }
    }
}