#include "draft/editor/panels/component_gizmo.hpp"
#include "draft/ecs/component_catalog.hpp"
#include "draft/ecs/gizmo_context.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/panels/gizmo_primitives.hpp"
#include "draft/rendering/camera.hpp"

#include "imgui.h"

namespace Draft {
    ComponentGizmoSystem::ComponentGizmoSystem(EditorApplication& app) : m_app(app) {}

    RenderLayer ComponentGizmoSystem::get_render_layers() const { return RenderLayer::Interface; }

    void ComponentGizmoSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Interface || !m_app.viewportPanelVisible)
            return;

        Camera* camera = m_app.gameScene.get_active_camera();
        Entity selected = m_app.selection.get();

        if(!camera || !selected.is_valid())
            return;

        GizmoViewport viewport{*camera, m_app.viewportScreenPosition, m_app.viewportSize};
        FieldContext fieldCtx{ m_app.gameScene, m_app.assets, m_app.selection, m_app };
        GizmoContext gizmoCtx{ fieldCtx, viewport };

        if(ImGui::Begin("Viewport###Viewport")){
            for(ComponentTypeInterface* entry : m_app.gameEngine.components().all())
                if(entry->has(selected) && entry->has_custom_gizmo())
                    entry->draw_gizmo(selected, gizmoCtx);
        }

        ImGui::End();
    }
}
