#include "draft/editor/panels/inspector_panel.hpp"
#include "draft/ecs/component_catalog.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/field_widgets.hpp"
#include "draft/util/serialization/context.hpp"

#include "imgui.h"

namespace Draft {
    namespace {
        /**
         * @brief Bridges Runtime's type-erased FieldVisitor callback into field_widgets.hpp's
         * typed dispatch, holding just what one component's worth of field-drawing needs: the
         * shared FieldContext, and that component's own JSON serialization for the fallback
         * display of any field type not otherwise recognized.
         */
        class FieldDrawVisitor : public FieldVisitor {
        public:
            FieldDrawVisitor(FieldContext& ctx, const JSON& fallbackJson) : m_ctx(ctx), m_fallbackJson(fallbackJson) {}

            void visit(std::string_view name, std::type_index type, void* valuePtr) override {
                draw_typeerased_field(m_ctx, name, type, valuePtr, m_fallbackJson);
            }

        private:
            FieldContext& m_ctx;
            const JSON& m_fallbackJson;
        };
    }

    InspectorPanelSystem::InspectorPanelSystem(EditorApplication& app) : m_app(app) {}

    void InspectorPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        ImGui::SetNextWindowSize({320, 480}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Inspector")){
            Entity selected = m_app.selection.get();

            if(!selected.is_valid()){
                ImGui::TextDisabled("No entity selected");
            } else {
                // entry->serialize()/visit_fields() below can reach Entity or Resource<T> fields
                SceneSerializationContext serializationCtx;
                serializationCtx.assets = &m_app.assets;

                for(entt::entity raw : m_app.gameScene.get_registry().storage<entt::entity>()){
                    if(!m_app.gameScene.get_registry().valid(raw))
                        continue;

                    serializationCtx.entityToId[raw] = static_cast<uint32_t>(serializationCtx.idToEntity.size());
                    serializationCtx.idToEntity.push_back(Entity(&m_app.gameScene, raw));
                }

                Serializer::ScopedContext<SceneSerializationContext> scope(serializationCtx);

                for(ComponentTypeInterface* entry : m_app.gameEngine.components().all())
                    if(entry->has(selected))
                        draw_component_entry(*entry, selected);

                ImGui::Separator();

                if(ImGui::Button("Add Component", ImVec2(-1, 0)))
                    ImGui::OpenPopup("AddComponentPopup");

                draw_add_component_popup(selected);
            }
        }

        ImGui::End();
    }

    void InspectorPanelSystem::draw_component_entry(ComponentTypeInterface& entry, Entity entity){
        ImGui::PushID(entry.name().c_str());

        bool open = ImGui::CollapsingHeader(entry.name().c_str(), ImGuiTreeNodeFlags_DefaultOpen);
        bool removed = false;

        if(ImGui::BeginPopupContextItem()){
            if(ImGui::MenuItem("Remove Component")){
                entry.remove(entity);
                removed = true;
            }

            ImGui::EndPopup();
        }

        if(open && !removed){
            JSON fallbackJson;
            entry.serialize(entity, fallbackJson);

            FieldContext ctx{ m_app.gameScene, m_app.assets, m_app.selection };
            FieldDrawVisitor visitor(ctx, fallbackJson);
            entry.visit_fields(entity, visitor);
        }

        ImGui::PopID();
    }

    void InspectorPanelSystem::draw_add_component_popup(Entity entity){
        if(ImGui::BeginPopup("AddComponentPopup")){
            for(ComponentTypeInterface* entry : m_app.gameEngine.components().all()){
                if(entry->has(entity))
                    continue;

                if(ImGui::MenuItem(entry->name().c_str())){
                    entry->add_default(entity);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }
}
