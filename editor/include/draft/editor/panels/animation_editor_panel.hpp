#pragma once

#include "draft/asset/resource.hpp"
#include "draft/ecs/system.hpp"
#include "draft/rendering/animation.hpp"
#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/frame_buffer.hpp"
#include "draft/util/reflectable.hpp"

#include <string>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Tabbed alongside the Viewport, invisible until AssetBrowserPanelSystem opens an
     * animation asset. Lists the animation's tags (if it has any) and plays the selected one
     * (or, with no tag selected/no tags at all, every frame in sequence) in a square preview.
     */
    class AnimationEditorPanelSystem : public AbstractSystem {
    public:
        explicit AnimationEditorPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(AnimationEditorPanelSystem)

    private:
        void load_asset(const std::string& key);
        void draw_tag_list(Animation& animation);
        void draw_preview(Time dt, Animation& animation, float availableHeight);

        EditorApplication& m_app;

        std::string m_loadedKey;
        Resource<Animation> m_resource;
        std::string m_selectedTag; // empty means "every frame", not any specific tag

        float m_frameTime = 0.f;
        Framebuffer m_previewTarget;
        SpriteCollection m_previewBatch;
        float m_previewExtent = 1.f;
    };
}
