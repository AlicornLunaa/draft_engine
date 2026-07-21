#include "draft/editor/panels/systems_panel.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/ecs/system_catalog.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/field_widgets.hpp"
#include "draft/util/logger.hpp"
#include "draft/util/serialization/context.hpp"

#include "imgui.h"

#include <exception>

namespace Draft {
    SystemsPanelSystem::SystemsPanelSystem(EditorApplication& app) : m_app(app) {}

    void SystemsPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default || !m_app.systemsPanelVisible)
            return;

        ImGui::SetNextWindowSize({320, 240}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Systems", &m_app.systemsPanelVisible)){
            SystemRegistry& registry = m_app.gameScene.get_systems();

            // entry->serialize()/visit_fields()/deserialize() below can reach a system's own
            // Entity or Resource<T> fields, same reason InspectorPanelSystem::render() sets this up.
            SceneSerializationContext serializationCtx;
            serializationCtx.assets = &m_app.assets;

            for(entt::entity raw : m_app.gameScene.get_registry().storage<entt::entity>()){
                if(!m_app.gameScene.get_registry().valid(raw))
                    continue;

                serializationCtx.entityToId[raw] = static_cast<uint32_t>(serializationCtx.idToEntity.size());
                serializationCtx.idToEntity.push_back(Entity(&m_app.gameScene, raw));
            }

            Serializer::ScopedContext<SceneSerializationContext> scope(serializationCtx);

            for(SystemTypeInterface* entry : m_app.gameEngine.systems().all()){
                ImGui::PushID(entry->name().c_str());

                bool attached = entry->has(registry);
                if(ImGui::Checkbox(entry->name().c_str(), &attached)){
                    if(attached)
                        entry->add(m_app.gameScene);
                    else
                        entry->remove(registry);
                }

                if(entry->has(registry) && entry->has_fields())
                    draw_system_config(*entry, registry);

                ImGui::PopID();
            }
        }

        ImGui::End();
    }

    void SystemsPanelSystem::draw_system_config(SystemTypeInterface& entry, SystemRegistry& registry){
        if(!ImGui::CollapsingHeader("Config"))
            return;

        ImGui::Indent();

        JSON systemJson;
        entry.serialize(registry, systemJson);

        FieldContext ctx{m_app.gameScene, m_app.assets, m_app.selection, m_app};
        FieldDrawVisitor visitor(ctx, systemJson);
        entry.visit_fields(registry, visitor);

        if(!visitor.changed_fallback_keys().empty()){
            JSON freshJson;
            entry.serialize(registry, freshJson);

            for(const std::string& key : visitor.changed_fallback_keys())
                freshJson[key] = systemJson[key];

            // A hand-edited JSON subtree can be malformed for whatever T's own deserialize expects
            try {
                entry.deserialize(registry, freshJson);
            } catch(const std::exception& e){
                Logger::println(LogLevel::Severe, "Systems", std::string("Failed to apply edit to ") + entry.name() + ": " + e.what());
            }
        }

        ImGui::Unindent();
    }
}
