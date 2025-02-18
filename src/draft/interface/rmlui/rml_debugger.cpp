#include "draft/interface/rmlui/rml_debugger.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/math/glm.hpp"

#include "RmlUi/Debugger/Debugger.h"

#include <cassert>
#include <stdexcept>

namespace Draft {
    // Static variables
    bool RmlDebugger::s_debuggerExists = false;
    
    // Constructors
    RmlDebugger::RmlDebugger(RmlEngine& engine, const Vector2i& size) : RmlContext(engine, "debugger", size) {
        // Error check
        if(s_debuggerExists)
            throw std::runtime_error("Only one RML debugger can exist at one moment");

        // Initialize debugger
        Rml::Debugger::Initialise(get_context());
        s_debuggerExists = true;
    }

    RmlDebugger::~RmlDebugger(){
        Rml::Debugger::Shutdown();
        s_debuggerExists = false;
    }
    
    // Functions
    bool RmlDebugger::handle_event(const Event& event){
        if(event.type == Event::KeyPressed && event.key.code == Keyboard::F4){
            set_visible(!is_visible());
            return true;
        }

        return RmlContext::handle_event(event);
    }
    
    void RmlDebugger::set_visible(bool visible){
        Rml::Debugger::SetVisible(visible);
    }

    bool RmlDebugger::is_visible() const {
        return Rml::Debugger::IsVisible();
    }

    void RmlDebugger::debug(Rml::Context* ctx){
        assert(ctx && "Context is not valid");
        Rml::Debugger::SetContext(ctx);
    }

    void RmlDebugger::debug(const RmlContext& ctx){
        Rml::Debugger::SetContext(ctx.get_context());
    }

    void RmlDebugger::debug(const RmlContext* ctx){
        assert(ctx && "Context is not valid");
        Rml::Debugger::SetContext(ctx->get_context());
    }
}