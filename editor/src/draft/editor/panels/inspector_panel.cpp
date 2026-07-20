#include "draft/editor/panels/inspector_panel.hpp"
#include "draft/ecs/component_catalog.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/field_widgets.hpp"
#include "draft/util/logger.hpp"
#include "draft/util/serialization/context.hpp"

#include "imgui.h"

#include <exception>

namespace Draft {
    namespace {
        /**
         * @brief Bridges Runtime's type-erased FieldVisitor callback into field_widgets.hpp's
         * typed dispatch, holding just what one component's worth of field-drawing needs: the
         * shared FieldContext, and that component's own JSON serialization used both as the
         * generic JSON-subtree editor's backing store for any field type with no typed widget
         */
        class FieldDrawVisitor : public FieldVisitor {
        public:
            FieldDrawVisitor(FieldContext& ctx, JSON& componentJson) : m_ctx(ctx), m_componentJson(componentJson) {}

            void visit(std::string_view name, std::type_index type, void* valuePtr) override {
                bool usedJsonFallback = false;
                bool changed = draw_typeerased_field(m_ctx, name, type, valuePtr, m_componentJson, usedJsonFallback);

                if(changed){
                    m_anyChanged = true;
                    if(usedJsonFallback)
                        m_changedFallbackKeys.emplace_back(name);
                }
            }

            const std::vector<std::string>& changed_fallback_keys() const { return m_changedFallbackKeys; }
            bool any_changed() const { return m_anyChanged; }

        private:
            FieldContext& m_ctx;
            JSON& m_componentJson;
            std::vector<std::string> m_changedFallbackKeys;
            bool m_anyChanged = false;
        };
    }

    InspectorPanelSystem::InspectorPanelSystem(EditorApplication& app) : m_app(app) {}

    void InspectorPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default || !m_app.inspectorPanelVisible)
            return;

        ImGui::SetNextWindowSize({320, 480}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Inspector", &m_app.inspectorPanelVisible)){
            Entity selected = m_app.selection.get();

            if(m_app.selection.count() > 1){
                // Multi-entity field editing isn't implemented, showing one arbitrary entity's
                // components while the label says "N entities" would misrepresent what Add/Remove
                // Component or a field edit is actually about to touch, so this stays read-only.
                ImGui::Text("%zu entities selected", m_app.selection.count());
                ImGui::Separator();

                for(Entity entity : m_app.selection.all())
                    ImGui::BulletText("%s", entity_label(entity).c_str());
            } else if(!selected.is_valid()){
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

            ImGui::Separator();

            if(ImGui::MenuItem("Copy JSON")){
                JSON json;
                entry.serialize(entity, json);
                ImGui::SetClipboardText(json.dump(4).c_str());
            }

            if(ImGui::MenuItem("Paste JSON")){
                const char* clipboard = ImGui::GetClipboardText();

                if(clipboard){
                    try {
                        JSON json = JSON::parse(clipboard);
                        entry.deserialize(entity, json);
                        entry.notify_modified(entity);
                    } catch(const std::exception& e){
                        Logger::println(LogLevel::Severe, "Inspector", std::string("Failed to paste JSON into ") + entry.name() + ": " + e.what());
                    }
                }
            }

            ImGui::EndPopup();
        }

        if(open && !removed){
            JSON componentJson;
            entry.serialize(entity, componentJson);

            FieldContext ctx{ m_app.gameScene, m_app.assets, m_app.selection, m_app };
            FieldDrawVisitor visitor(ctx, componentJson);
            entry.visit_fields(entity, visitor);

            if(!visitor.changed_fallback_keys().empty()){
                JSON freshJson;
                entry.serialize(entity, freshJson);

                for(const std::string& key : visitor.changed_fallback_keys())
                    freshJson[key] = componentJson[key];

                // A hand-edited JSON subtree can be malformed for whatever T's own deserialize expects
                try {
                    entry.deserialize(entity, freshJson);
                } catch(const std::exception& e){
                    Logger::println(LogLevel::Severe, "Inspector", std::string("Failed to apply edit to ") + entry.name() + ": " + e.what());
                }
            }

            // Fire ENTT's on modify hooks
            if(visitor.any_changed())
                entry.notify_modified(entity);
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
