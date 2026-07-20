#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

#include <array>
#include <string>
#include <vector>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Draws the edit scene's entity tree, following ParentComponent/ChildComponent.
     * Create, delete, reparent (drag drop), and select all act directly on the registry, no
     * shadow state of its own beyond which row is being dragged
     */
    class HierarchyPanelSystem : public AbstractSystem {
    public:
        explicit HierarchyPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(HierarchyPanelSystem)

    private:
        void create_entity();
        void draw_entity_row(Entity entity, std::vector<Entity>& entitiesToRemove, std::vector<Entity>& drawOrder);
        void handle_row_selection(Entity entity);
        void select_range(Entity anchor, Entity target);
        void reparent(Entity child, Entity newParent);
        Entity duplicate_entity(Entity source);
        Entity clone_subtree(Entity source, Entity parent);
        static std::string label_for(Entity entity);

        void open_save_prefab_prompt(Entity entity);
        void draw_save_prefab_modal();

        EditorApplication& m_app;
        Entity m_prefabSaveTarget;
        std::array<char, 512> m_prefabPathBuffer{};
        bool m_openSavePrefabPopupRequested = false;

        std::vector<Entity> m_drawOrder;
        Entity m_selectionAnchor;
    };
}
