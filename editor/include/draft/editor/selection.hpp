#pragma once

#include "draft/ecs/entity.hpp"

#include <functional>
#include <vector>

namespace Draft {
    /**
     * @brief Holds the editor's selected entities, in selection order. The last entity added is
     * the "primary" selection (get()): what the Inspector edits and what every single-entity-
     * oriented caller (asset drops, an Entity field's jump-to-selection button, scene reload)
     * uses. Panels that need to react to a selection change subscribe via on_change() instead of
     * polling every frame.
     */
    class EditorSelection {
    public:
        using Callback = std::function<void(Entity)>;

        /**
         * @brief The primary selection, or an invalid Entity if nothing is selected.
         */
        Entity get() const;

        /**
         * @brief Every selected entity, oldest first, primary (get()) last.
         */
        const std::vector<Entity>& all() const { return m_selected; }

        std::size_t count() const { return m_selected.size(); }
        bool is_selected(Entity entity) const;

        /**
         * @brief Replaces the selection with just @p entity, or clears it if @p entity is
         * invalid. Used by every caller that means "select this one thing."
         */
        void set(Entity entity);

        /**
         * @brief Replaces the selection with exactly @p entities, in the given order (the last
         * entry becomes primary()). Used for the hierarchy panel's Shift-click range-select.
         */
        void set_range(std::vector<Entity> entities);

        /**
         * @brief Adds @p entity to the selection (becoming the new primary()), or removes it if
         * already selected. Used for the hierarchy panel's Ctrl-click.
         */
        void toggle(Entity entity);

        void clear();

        void on_change(Callback callback);

    private:
        void notify();

        std::vector<Entity> m_selected;
        std::vector<Callback> m_callbacks;
    };
}
