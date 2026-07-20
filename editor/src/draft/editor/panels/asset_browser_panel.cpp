#include "draft/editor/panels/asset_browser_panel.hpp"
#include "draft/editor/asset_drag_drop.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/project.hpp"

#include "imgui.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace Draft {
    AssetBrowserPanelSystem::AssetBrowserPanelSystem(EditorApplication& app) : m_app(app) {}

    void AssetBrowserPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        ImGui::SetNextWindowSize({320, 320}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Asset Browser")){
            if(!m_app.has_project()){
                ImGui::TextDisabled("No project open");
            } else {
                if(m_scannedRoot != m_app.project()->root())
                    rescan();

                if(ImGui::SmallButton("Refresh"))
                    rescan();

                ImGui::Separator();
                draw_node(m_root);
            }
        }

        ImGui::End();
    }

    void AssetBrowserPanelSystem::rescan(){
        m_scannedRoot = m_app.project()->root();
        m_root = AssetNode{};
        m_root.isDirectory = true;

        for(const AssetTask& task : collect_project_assets(m_scannedRoot)){
            AssetNode* cursor = &m_root;
            std::filesystem::path relativeKey(task.key);

            for(auto it = relativeKey.begin(); it != relativeKey.end(); ++it){
                std::string part = it->string();
                AssetNode& child = cursor->children[part];
                child.name = part;

                bool isLeaf = (std::next(it) == relativeKey.end());
                if(isLeaf){
                    child.key = task.key;
                    child.kind = task.kind;
                    child.isDirectory = false;
                } else {
                    child.isDirectory = true;
                }

                cursor = &child;
            }
        }
    }

    void AssetBrowserPanelSystem::draw_node(const AssetNode& node){
        std::vector<const AssetNode*> sorted;
        sorted.reserve(node.children.size());
        for(const auto& [name, child] : node.children)
            sorted.push_back(&child);

        // Folders first, then files grouped by kind (alphabetically by kind name), then
        // alphabetically by filename within a kind.
        std::sort(sorted.begin(), sorted.end(), [](const AssetNode* a, const AssetNode* b){
            if(a->isDirectory != b->isDirectory)
                return a->isDirectory;

            if(!a->isDirectory && a->kind != b->kind)
                return std::strcmp(asset_kind_name(a->kind), asset_kind_name(b->kind)) < 0;

            return a->name < b->name;
        });

        for(const AssetNode* childPtr : sorted){
            const AssetNode& child = *childPtr;

            if(child.isDirectory){
                if(ImGui::TreeNodeEx(child.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)){
                    draw_node(child);
                    ImGui::TreePop();
                }

                continue;
            }

            std::string label = child.name + "##" + child.key;
            if(ImGui::Selectable(label.c_str(), m_selectedKey == child.key))
                m_selectedKey = child.key;

            if(ImGui::IsItemHovered()){
                ImGui::SetTooltip("%s", asset_kind_name(child.kind));

                if(child.kind == AssetKind::Scene && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    open_scene(child.key);
            }

            if(ImGui::BeginDragDropSource()){
                ImGui::SetDragDropPayload(asset_drag_payload_type(child.kind), child.key.data(), child.key.size());
                ImGui::Text("%s", child.name.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }

    void AssetBrowserPanelSystem::open_scene(const std::string& key){
        if(m_app.is_playing())
            return;

        m_app.request_open_scene(m_scannedRoot / key);
    }
}
