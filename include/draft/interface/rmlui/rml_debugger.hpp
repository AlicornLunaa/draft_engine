#pragma once

#include "draft/interface/rmlui/rml_context.hpp"
#include "draft/math/glm.hpp"

#include "RmlUi/Core/Context.h"

namespace Draft {
    class RmlDebugger : public RmlContext {
    private:
        // Static variables
        static bool s_debuggerExists;
        
    public:
        // Constructors
        RmlDebugger(RmlEngine& engine, const Vector2i& size);
        ~RmlDebugger();

        // Functions
        bool handle_event(const Event& event) override;

        void set_visible(bool visible);
        bool is_visible() const;

        void debug(Rml::Context* ctx);
        void debug(const RmlContext& ctx);
        void debug(const RmlContext* ctx);
    };
}