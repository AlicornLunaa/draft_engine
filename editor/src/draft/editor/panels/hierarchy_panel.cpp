#include "draft/editor/panels/hierarchy_panel.hpp"
#include "draft/components/tag_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/ecs/relationship_components.hpp"
#include "draft/editor/editor_application.hpp"

#include "imgui.h"

#include <vector>

namespace Draft {
    HierarchyPanelSystem::HierarchyPanelSystem(EditorApplication& app) : m_app(app) {}

    void HierarchyPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        ImGui::SetNextWindowSize({64, 480}, ImGuiCond_FirstUseEver);

        ImGui::Begin("Hierarchy");

        if(ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)){
            if(ImGui::MenuItem("Create Entity"))
                create_entity();
            ImGui::EndPopup();
        }

        Registry& registry = m_app.gameScene.get_registry();
        std::vector<Entity> roots;
        for(entt::entity raw : registry.storage<entt::entity>()){
            Entity entity(&m_app.gameScene, raw);
            if(!entity.has_component<ChildComponent>())
                roots.push_back(entity);
        }

        for(Entity root : roots)
            draw_entity_row(root);

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

        ImGui::End();
    }

    void HierarchyPanelSystem::create_entity(){
        Entity entity = m_app.gameScene.create_entity();
        entity.add_component<TagComponent>(TagComponent{"Entity"});
        entity.add_component<TransformComponent>();
        m_app.selection.set(entity);
    }

    void HierarchyPanelSystem::draw_entity_row(Entity entity){
        if(!entity.is_valid())
            return;

        auto* parentComp = entity.try_get_component<ParentComponent>();
        bool hasChildren = parentComp && !parentComp->children.empty();

        // Snapshot the children now
        std::vector<Entity> children = hasChildren ? parentComp->children : std::vector<Entity>();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if(!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if(m_app.selection.get() == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

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

        bool destroyed = false;
        if(ImGui::BeginPopupContextItem()){
            if(ImGui::MenuItem("Delete")){
                bool wasSelected = m_app.selection.get() == entity;
                entity.destroy();
                destroyed = true;

                // Destroying entity may cascade-destroy descendants, clear the selection if it
                // was the deleted row itself or if it was one of those descendants.
                if(wasSelected || !m_app.selection.get().is_valid())
                    m_app.selection.clear();
            }
            ImGui::EndPopup();
        }

        if(open && hasChildren){
            if(!destroyed)
                for(Entity child : children)
                    draw_entity_row(child);

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

    std::string HierarchyPanelSystem::label_for(Entity entity){
        if(auto* tag = entity.try_get_component<TagComponent>(); tag && !tag->tag.empty())
            return tag->tag;

        return "Entity " + std::to_string(static_cast<uint32_t>(static_cast<entt::entity>(entity)));
    }
}
