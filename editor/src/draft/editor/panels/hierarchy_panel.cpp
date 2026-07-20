#include "draft/editor/panels/hierarchy_panel.hpp"
#include "draft/components/tag_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/ecs/component_catalog.hpp"
#include "draft/ecs/relationship_components.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/prefab.hpp"

#include "imgui.h"

#include <cstring>
#include <typeindex>
#include <vector>

namespace Draft {
    HierarchyPanelSystem::HierarchyPanelSystem(EditorApplication& app) : m_app(app) {}

    void HierarchyPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        std::vector<Entity> entitiesToRemove;

        ImGui::SetNextWindowSize({64, 480}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Hierarchy")){
            // Context popup for an entity on empty space
            if(ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)){
                if(ImGui::MenuItem("Create Entity"))
                    create_entity();

                ImGui::EndPopup();
            }
    
            Registry& registry = m_app.gameScene.get_registry();

            std::vector<Entity> rootEntities;

            // Collect all top level entities
            for(entt::entity raw : registry.storage<entt::entity>()){
                Entity entity(&m_app.gameScene, raw);

                if(entity && !entity.has_component<ChildComponent>()){
                    // Draw each root entity
                    draw_entity_row(entity, entitiesToRemove);
                }
            }
    
            // Fills the rest of the panel so dropping on empty space unparents (drops to root).
            ImGui::Dummy(ImGui::GetContentRegionAvail());
            if(ImGui::BeginDragDropTarget()){
                if(const ImGuiPayload* incoming = ImGui::AcceptDragDropPayload("HierarchyEntity")){
                    Entity dragged = *static_cast<const Entity*>(incoming->Data);
                    if(dragged.is_valid() && dragged.has_component<ChildComponent>())
                        dragged.remove_component<ChildComponent>();
                }

                ImGui::EndDragDropTarget();
            }
        }

        ImGui::End();

        if(m_openSavePrefabPopupRequested){
            m_openSavePrefabPopupRequested = false;
            ImGui::OpenPopup("Save As Prefab");
        }

        draw_save_prefab_modal();

        // Cleanup
        for(auto entity : entitiesToRemove){
            entity.destroy();
        }
    }

    void HierarchyPanelSystem::create_entity(){
        Entity entity = m_app.gameScene.create_entity();
        entity.add_component<TagComponent>(TagComponent{"Entity"});
        entity.add_component<TransformComponent>();
        m_app.selection.set(entity);
    }

    void HierarchyPanelSystem::draw_entity_row(Entity entity, std::vector<Entity>& entitiesToRemove){
        if(!entity.is_valid())
            return;

        auto* parentComp = entity.try_get_component<ParentComponent>();
        bool hasChildren = parentComp && !parentComp->children.empty();

        // Snapshot the children now
        std::vector<Entity> children = hasChildren ? parentComp->children : std::vector<Entity>();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DrawLinesToNodes;
        if(!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if(m_app.selection.get() == entity) flags |= ImGuiTreeNodeFlags_Selected;

        std::string label = label_for(entity);
        auto id = reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<entt::entity>(entity)));
        bool open = ImGui::TreeNodeEx(id, flags, "%s", label.c_str());

        if(ImGui::IsItemClicked())
            m_app.selection.set(entity);

        if(ImGui::BeginDragDropSource()){
            Entity payload = entity;
            ImGui::SetDragDropPayload("HierarchyEntity", &payload, sizeof(Entity));
            ImGui::Text("%s", label.c_str());
            ImGui::EndDragDropSource();
        }

        if(ImGui::BeginDragDropTarget()){
            if(const ImGuiPayload* incoming = ImGui::AcceptDragDropPayload("HierarchyEntity")){
                Entity dragged = *static_cast<const Entity*>(incoming->Data);
                reparent(dragged, entity);
            }

            ImGui::EndDragDropTarget();
        }

        if(ImGui::BeginPopupContextItem()){
            if(ImGui::MenuItem("Duplicate"))
                m_app.selection.set(duplicate_entity(entity));

            if(ImGui::MenuItem("Save As Prefab..."))
                open_save_prefab_prompt(entity);

            if(ImGui::MenuItem("Delete")){
                bool wasSelected = m_app.selection.get() == entity;
                entitiesToRemove.push_back(entity);

                // Destroying entity may cascade-destroy descendants, clear the selection if it
                // was the deleted row itself or if it was one of those descendants.
                if(wasSelected || !m_app.selection.get().is_valid())
                    m_app.selection.clear();
            }

            ImGui::EndPopup();
        }

        if(open && hasChildren){
            for(Entity child : children)
                draw_entity_row(child, entitiesToRemove);

            ImGui::TreePop();
        }
    }

    void HierarchyPanelSystem::reparent(Entity child, Entity newParent){
        if(!child.is_valid() || child == newParent)
            return;

        // Refuse to make an entity its own descendant.
        for(Entity cursor = newParent; cursor.is_valid(); ){
            if(cursor == child)
                return;

            auto* childComp = cursor.try_get_component<ChildComponent>();
            cursor = childComp ? childComp->parent : Entity();
        }

        if(child.has_component<ChildComponent>())
            child.remove_component<ChildComponent>();

        child.add_component<ChildComponent>(ChildComponent{newParent});
    }

    Entity HierarchyPanelSystem::duplicate_entity(Entity source){
        if(!source.is_valid())
            return Entity();

        auto* childComp = source.try_get_component<ChildComponent>();
        return clone_subtree(source, childComp ? childComp->parent : Entity());
    }

    Entity HierarchyPanelSystem::clone_subtree(Entity source, Entity parent){
        Entity duplicate = m_app.gameScene.create_entity();

        // ChildComponent/ParentComponent point at specific entities, so a generic clone() would
        // link the duplicate to the original's relationships instead of its own.
        std::type_index childType(typeid(ChildComponent));
        std::type_index parentType(typeid(ParentComponent));

        for(ComponentTypeInterface* entry : m_app.gameEngine.components().all()){
            if(entry->type() == childType || entry->type() == parentType)
                continue;

            if(entry->has(source))
                entry->clone(source, duplicate);
        }

        if(parent.is_valid())
            duplicate.add_component<ChildComponent>(ChildComponent{parent});

        if(auto* parentComp = source.try_get_component<ParentComponent>())
            for(Entity child : parentComp->children)
                clone_subtree(child, duplicate);

        return duplicate;
    }

    std::string HierarchyPanelSystem::label_for(Entity entity){
        if(auto* tag = entity.try_get_component<TagComponent>(); tag && !tag->tag.empty())
            return tag->tag;

        return "Entity " + std::to_string(static_cast<uint32_t>(static_cast<entt::entity>(entity)));
    }

    void HierarchyPanelSystem::open_save_prefab_prompt(Entity entity){
        m_prefabSaveTarget = entity;

        std::filesystem::path suggested = m_app.project()->assets_dir() / "prefabs" / (label_for(entity) + ".prefab");
        std::string suggestedStr = suggested.string();
        std::strncpy(m_prefabPathBuffer.data(), suggestedStr.c_str(), m_prefabPathBuffer.size() - 1);

        m_openSavePrefabPopupRequested = true;
    }

    void HierarchyPanelSystem::draw_save_prefab_modal(){
        if(ImGui::BeginPopupModal("Save As Prefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
            ImGui::SetNextItemWidth(400.f);
            ImGui::InputText("##PrefabPath", m_prefabPathBuffer.data(), m_prefabPathBuffer.size());

            if(ImGui::Button("Save")){
                if(m_prefabSaveTarget.is_valid())
                    PrefabManager(m_app).save_prefab(m_prefabSaveTarget, std::filesystem::path(m_prefabPathBuffer.data()));

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }
}
