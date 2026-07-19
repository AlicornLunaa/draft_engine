#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;
    struct ComponentTypeInterface;

    /**
     * @brief Draws the selected entity's components with one collapsible section per component the
     * entity has(). Each rendered generically through ComponentTypeInterface::visit_fields()/field_widgets.hpp
     * Plus an Add Component popup listing catalog entries the entity doesn't already have.
     */
    class InspectorPanelSystem : public AbstractSystem {
    public:
        explicit InspectorPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(InspectorPanelSystem)

    private:
        void draw_component_entry(ComponentTypeInterface& entry, Entity entity);
        void draw_add_component_popup(Entity entity);

        EditorApplication& m_app;
    };
}
