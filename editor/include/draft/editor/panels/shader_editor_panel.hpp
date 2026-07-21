#pragma once

#include "draft/asset/resource.hpp"
#include "draft/ecs/system.hpp"
#include "draft/editor/game_module_watcher.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_array.hpp"
#include "draft/util/reflectable.hpp"

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Tabbed alongside the Viewport, invisible until AssetBrowserPanelSystem opens a
     * shader asset (a directory directly containing vertex.glsl + fragment.glsl). Compiles the
     * shader live and renders it on a 4-vertex dummy quad, its vertex attributes and loose
     * uniforms reflected from the linked program (see Shader::reflect_attributes()/
     * reflect_uniforms()) and fed from ImGui-editable mock values. Mock values persist per
     * shader under .draft-editor/shader_previews.
     *
     * Recompiles automatically whenever either GLSL file changes on disk (polled the same way
     * EditorApplication polls its own GameModuleWatcher). A failed compile keeps showing the
     * last successfully compiled shader with its reflected data untouched - Shader itself logs
     * the failure, this panel additionally keeps the message around for inline display.
     */
    class ShaderEditorPanelSystem : public AbstractSystem {
    public:
        explicit ShaderEditorPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(ShaderEditorPanelSystem)

    private:
        // One editable mock value, either a vertex attribute at one of the 4 dummy vertices or
        // a uniform. Component count/meaning depends on the ShaderDataType it belongs to -
        // unused trailing components stay zero and are ignored.
        using MockValue = std::array<float, 4>;

        void load_asset(const std::string& key);
        void compile_and_reflect();
        void reconcile_mock_data();
        const ShaderAttribute* find_attribute(const std::string& name) const;

        void rebuild_preview_mesh();
        std::vector<float> flatten_attribute(const std::string& name, int componentCount) const;
        void upload_attribute(const ShaderAttribute& attribute);
        void apply_uniforms() const;

        std::filesystem::path mock_data_path() const;
        void load_mock_data();
        void save_mock_data() const;

        void draw_preview(float availableHeight);
        void draw_attribute_editors();
        void draw_uniform_editors();

        EditorApplication& m_app;

        std::string m_loadedKey;
        std::optional<Shader> m_shader;
        std::string m_compileError;

        std::optional<GameModuleWatcher> m_vertexWatcher;
        std::optional<GameModuleWatcher> m_fragmentWatcher;

        std::vector<ShaderAttribute> m_attributes;
        std::vector<ShaderUniform> m_uniforms;

        // Detected but never fed - Draft has no generic way to mock a storage block's contents
        // (see ShaderStorageBlock's doc comment). Kept only to show an inline warning instead of
        // silently rendering whatever an unbound binding point happens to read back as.
        std::vector<ShaderStorageBlock> m_storageBlocks;

        std::unordered_map<std::string, std::array<MockValue, 4>> m_attributeValues;
        std::unordered_map<std::string, MockValue> m_uniformValues;
        std::unordered_map<std::string, Resource<Texture>> m_samplerValues;

        // Built by rebuild_preview_mesh() from m_attributes, one StaticBuffer per supported
        // attribute in this same order (index into both matches m_vaoAttributeOrder).
        std::optional<VertexArray> m_previewVao;
        std::vector<std::string> m_vaoAttributeOrder;

        Framebuffer m_previewTarget;
        bool m_wireframe = false;
    };
}
