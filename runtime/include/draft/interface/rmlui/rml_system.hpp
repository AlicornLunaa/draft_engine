#pragma once

#include "draft/ecs/system.hpp"

#include <vector>

class SystemInterface_GLFW;
class RenderInterface_GL3;

namespace Draft {
    /// Forward decls
    class RenderWindow;
    class RmlContext;

    /**
     * @brief Owns the global RmlUi backend (SystemInterface_GLFW + RenderInterface_GL3).
     * Ref-counted so more than one RmlUiSystem/window pair can coexist in the same process.
     *
     * Doesn't own any Rml::Context itself, RmlContext is still an independently-constructible
     * wrapper and self-registers/unregisters into this system's m_contexts on construction/destruction.
     */
    class RmlUiSystem : public AbstractSystem {
    private:
        // Statics
        static int s_backendCount;
        static SystemInterface_GLFW* s_systemInterface;
        static RenderInterface_GL3* s_renderInterface;

        // Variables
        std::vector<RmlContext*> m_contexts;

        friend class RmlContext;
        void register_context(RmlContext& context);
        void unregister_context(RmlContext& context);

    public:
        // Constructors
        RmlUiSystem(RenderWindow& window);
        RmlUiSystem(const RmlUiSystem& other) = delete;
        RmlUiSystem(RmlUiSystem&& other) = delete;
        ~RmlUiSystem() override;

        // Operators
        RmlUiSystem& operator=(const RmlUiSystem& other) = delete;
        RmlUiSystem& operator=(RmlUiSystem&& other) = delete;

        // Functions
        RenderLayer get_render_layers() const override { return RenderLayer::Overlay; }
        void render(Time dt, RenderLayer layer) override;
        bool on_event(const Event& event) override;
    };
}
