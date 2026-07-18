#pragma once

#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

#include <memory>
#include <vector>

class SystemInterface_GLFW;
class RenderInterface_GL3;

namespace Draft {
    /// Forward decls
    class RenderWindow;
    class RmlContext;
    class RmlDebugger;
    class RmlFileInterface;

    /**
     * @brief Owns the global RmlUi backend (SystemInterface_GLFW + RenderInterface_GL3 + RmlFileInterface).
     * Ref-counted so more than one RmlUiSystem/window pair can coexist in the same process.
     */
    class RmlUiSystem : public AbstractSystem {
    private:
        // Statics
        static int s_backendCount;
        static SystemInterface_GLFW* s_systemInterface;
        static RenderInterface_GL3* s_renderInterface;
        static RmlFileInterface* s_fileInterface;

        // Variables
        std::vector<std::unique_ptr<RmlContext>> m_contextPtrs;

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
        RmlDebugger& add_debugger(const Vector2i& size);
        RmlContext& add_context(const std::string& name, const Vector2i& size);
        void remove_context(RmlContext& context);

        RenderLayer get_render_layers() const override { return RenderLayer::Overlay; }
        void render(Time dt, RenderLayer layer) override;
        bool on_event(const Event& event) override;
        bool wants_keyboard_capture() const; // True if any owned context has a text-editable element focused
        bool wants_mouse_capture() const; // True if the mouse is hovering an element in any owned context

        DRAFT_REFLECTABLE(RmlUiSystem);
    };
}
