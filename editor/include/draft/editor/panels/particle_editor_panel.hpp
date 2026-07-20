#pragma once

#include "draft/asset/resource.hpp"
#include "draft/ecs/system.hpp"
#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/rendering/particle_system.hpp"
#include "draft/util/reflectable.hpp"

#include <string>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Tabbed alongside the Viewport, invisible until AssetBrowserPanelSystem opens a
     * .particle asset. Shows a live looping demo of the emitter next to every one of
     * ParticleProps's reflected fields, and a Save button writing edits back to the file.
     */
    class ParticleEditorPanelSystem : public AbstractSystem {
    public:
        explicit ParticleEditorPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(ParticleEditorPanelSystem)

    private:
        void load_asset(const std::string& key);
        void save_asset();
        void draw_preview(Time dt, ParticleProps& props, float availableHeight);
        void draw_properties(ParticleProps& props);

        EditorApplication& m_app;

        std::string m_loadedKey;
        Resource<ParticleProps> m_resource;

        ParticleSystem m_particleSystem;
        Framebuffer m_previewTarget;
        SpriteCollection m_previewBatch;
        float m_emitAccumulator = 0.f;
        float m_previewExtent = 5.f;
    };
}
