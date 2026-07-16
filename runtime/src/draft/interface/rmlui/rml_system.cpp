#include "draft/interface/rmlui/rml_system.hpp"
#include "draft/interface/rmlui/RmlUi_Platform_GLFW.h"
#include "draft/interface/rmlui/RmlUi_Renderer_GL3.h"
#include "draft/interface/rmlui/rml_context.hpp"
#include "draft/interface/rmlui/rml_debugger.hpp"
#include "draft/rendering/render_window.hpp"

#include "RmlUi/Core/Core.h"

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
        m_contextPtrs.clear();

        s_backendCount--;

        if(s_backendCount <= 0){
            Rml::Shutdown();

            delete s_renderInterface;
            delete s_systemInterface;
            s_renderInterface = nullptr;
            s_systemInterface = nullptr;
        }
    }

    // Functions
    RmlDebugger& RmlUiSystem::add_debugger(const Vector2i& size){
        auto debuggerPtr = std::make_unique<RmlDebugger>(size);
        auto& debuggerRef = *debuggerPtr;
        m_contextPtrs.push_back(std::move(debuggerPtr));
        return debuggerRef;
    }

    RmlContext& RmlUiSystem::add_context(const std::string& name, const Vector2i& size){
        m_contextPtrs.push_back(std::make_unique<RmlContext>(name, size));
        return *m_contextPtrs.back();
    }

    void RmlUiSystem::remove_context(RmlContext& context){
        for(auto it = m_contextPtrs.begin(); it != m_contextPtrs.end(); ++it){
            if(it->get() == &context){
                m_contextPtrs.erase(it);
                break;
            }
        }
    }
    
    void RmlUiSystem::render(Time, RenderLayer){
        s_renderInterface->BeginFrame();

        for(auto& context : m_contextPtrs)
            context->render();

        s_renderInterface->EndFrame();
    }

    bool RmlUiSystem::on_event(const Event& event){
        for(auto& context : m_contextPtrs){
            if(context->handle_event(event))
                return true;
        }

        return false;
    }
}
