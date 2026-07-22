#pragma once

#include "draft/ecs/system.hpp"
#include "draft/editor/game_module_watcher.hpp"
#include "draft/interface/rmlui/rml_context.hpp"
#include "draft/interface/rmlui/rml_system.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/util/json.hpp"
#include "draft/util/reflectable.hpp"

#include "RmlUi/Core/DataModelHandle.h"
#include "RmlUi/Core/ElementDocument.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Renders the document into its own offscreen RmlUiSystem/RmlContext, fed by a freely-authored
     * mock data model instead of whatever the game would normally bind. Mock values and canvas
     * settings persist per document under .draft-editor/rml_previews.
     *
     * Reloads automatically whenever the document or any .rcss it <link>s changes on disk.
     */
    class RmlEditorPanelSystem : public AbstractSystem {
    public:
        explicit RmlEditorPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(RmlEditorPanelSystem)

    private:
        void load_asset(const std::string& key);
        void reload_document();
        void rebuild_watchers();
        std::vector<std::filesystem::path> discover_rcss_links(const std::filesystem::path& rmlPath) const;
        void rebuild_data_model();

        void apply_data_model_text();
        void sync_data_model_text(); // m_dataModelText <- pretty-printed m_dataModel, after a load/reload

        std::filesystem::path mock_data_path() const;
        void load_mock_data();
        void save_mock_data() const;

        void draw_preview(float availableHeight);
        void draw_canvas_settings();
        void draw_data_model_editor();

        EditorApplication& m_app;

        std::string m_loadedKey;
        std::string m_loadError;

        RmlUiSystem m_rmlUi;
        RmlContext* m_context = nullptr;
        Rml::ElementDocument* m_document = nullptr;
        Rml::DataModelHandle m_dataModelHandle;

        std::optional<GameModuleWatcher> m_documentWatcher;
        std::vector<GameModuleWatcher> m_dependencyWatchers;

        bool m_useScreenSize = true;
        Vector2u m_customCanvasSize{800, 600};

        JSON m_dataModel = JSON::object();
        std::string m_dataModelText;
        std::string m_dataModelParseError;

        Framebuffer m_previewTarget;
    };
}
