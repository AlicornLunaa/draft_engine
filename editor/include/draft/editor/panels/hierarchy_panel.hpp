#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

#include <string>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Draws the edit scene's entity tree, following ParentComponent/ChildComponent.
     * Create, delete, reparent (drag drop), and select all act directly on the registry, no
     * shadow state of its own beyond which row is being dragged.
     */
    class HierarchyPanelSystem : public AbstractSystem {
    public:
        explicit HierarchyPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(HierarchyPanelSystem)

    private:
        void create_entity();
        void draw_entity_row(Entity entity, std::vector<Entity>& entitiesToRemove);
        void reparent(Entity child, Entity newParent);
        static std::string label_for(Entity entity);

        EditorApplication& m_app;
    };
}
