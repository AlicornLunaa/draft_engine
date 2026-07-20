#pragma once

#include "draft/build_tools/asset_pipeline.hpp"
#include "draft/ecs/system.hpp"

#include <filesystem>
#include <map>
#include <string>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Browses <project>/assets as a tree, classified via asset_pipeline.hpp
     */
    class AssetBrowserPanelSystem : public AbstractSystem {
    public:
        explicit AssetBrowserPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(AssetBrowserPanelSystem)

    private:
        struct AssetNode {
            std::string name;
            std::string key; // project-root-relative, only meaningful when !isDirectory
            AssetKind kind = AssetKind::Unknown;
            bool isDirectory = false;
            std::map<std::string, AssetNode> children;
        };

        void rescan();
        void draw_node(const AssetNode& node);
        void open_scene(const std::string& key);
        void instantiate_prefab_at_origin(const std::string& key);
        void open_particle_editor(const std::string& key);
        void reload_all();
        void reload_asset(AssetKind kind, const std::string& key);

        EditorApplication& m_app;
        std::filesystem::path m_scannedRoot;
        AssetNode m_root;
        std::string m_selectedKey;
    };
}
