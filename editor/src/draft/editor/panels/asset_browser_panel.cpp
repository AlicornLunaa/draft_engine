#include "draft/editor/panels/asset_browser_panel.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/editor/asset_drag_drop.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/prefab.hpp"
#include "draft/editor/project.hpp"
#include "draft/rendering/animation.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/texture.hpp"

#include "imgui.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace Draft {
    namespace {
        // Re-queues the key if it's already loaded, otherwise loads it fresh
        template<typename T>
        void reload_or_queue(AssetManager& assets, const std::string& key){
            if(!assets.reload<T>(key))
                assets.queue<T>(key);
        }

        // Scene/RML/RCSS/Language assets aren't loaded through AssetManager (see
        // asset_pipeline.cpp's validate_assets()), so there's nothing to reload for them.
        bool asset_kind_reloadable(AssetKind kind){
            switch(kind){
                case AssetKind::Texture:
                case AssetKind::Font:
                case AssetKind::Model:
                case AssetKind::Sound:
                case AssetKind::Animation:
                    return true;
                default:
                    return false;
            }
        }
    }

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

                ImGui::SameLine();
                if(ImGui::SmallButton("Reload All"))
                    reload_all();

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

                if(child.kind == AssetKind::Prefab && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    instantiate_prefab_at_origin(child.key);
            }

            if(ImGui::BeginDragDropSource()){
                ImGui::SetDragDropPayload(asset_drag_payload_type(child.kind), child.key.data(), child.key.size());
                ImGui::Text("%s", child.name.c_str());
                ImGui::EndDragDropSource();
            }

            if(ImGui::BeginPopupContextItem(label.c_str())){
                ImGui::BeginDisabled(!asset_kind_reloadable(child.kind));
                if(ImGui::MenuItem("Reload")){
                    reload_asset(child.kind, child.key);
                    m_app.assets.load();
                }
                ImGui::EndDisabled();

                ImGui::EndPopup();
            }
        }
    }

    void AssetBrowserPanelSystem::open_scene(const std::string& key){
        if(m_app.is_playing())
            return;

        m_app.request_open_scene(m_scannedRoot / key);
    }

    void AssetBrowserPanelSystem::instantiate_prefab_at_origin(const std::string& key){
        Entity instance = PrefabManager(m_app).instantiate_prefab(m_scannedRoot / key, Vector2f(0.f, 0.f));

        if(instance.is_valid())
            m_app.selection.set(instance);
    }

    void AssetBrowserPanelSystem::reload_all(){
        for(const AssetTask& task : collect_project_assets(m_scannedRoot))
            reload_asset(task.kind, task.key);

        m_app.assets.load();
    }

    void AssetBrowserPanelSystem::reload_asset(AssetKind kind, const std::string& key){
        switch(kind){
            case AssetKind::Texture: reload_or_queue<Texture>(m_app.assets, key); break;
            case AssetKind::Font: reload_or_queue<Font>(m_app.assets, key); break;
            case AssetKind::Model: reload_or_queue<Model>(m_app.assets, key); break;
            case AssetKind::Sound: reload_or_queue<SoundBuffer>(m_app.assets, key); break;
            case AssetKind::Animation: reload_or_queue<Animation>(m_app.assets, key); break;
            default: break;
        }
    }
}
