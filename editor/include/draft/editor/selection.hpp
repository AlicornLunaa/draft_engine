#pragma once

#include "draft/ecs/entity.hpp"

#include <functional>
#include <vector>

namespace Draft {
    /**
     * @brief Holds the editor's single selected entity. Panels that need to react to a
     * selection change (inspector, viewport gizmo) subscribe via on_change() instead of
     * polling every frame.
     */
    class EditorSelection {
    public:
        using Callback = std::function<void(Entity)>;

        Entity get() const { return m_selected; }
        void set(Entity entity);
        void clear();

        void on_change(Callback callback);

    private:
        Entity m_selected;
        std::vector<Callback> m_callbacks;
    };
}
