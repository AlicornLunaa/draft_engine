#pragma once

#include "draft/ecs/registry.hpp"
#include "draft/ecs/system.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief Submits every <SpriteComponent, TransformComponent> entity into the owning
     * Renderer's sprite batch every frame. Handles AnimationComponent overrides for SpriteComponent
     */
    class RenderSystem : public AbstractSystem {
    private:
        // Variables
        Registry& registryRef;
        Renderer& rendererRef;

    public:
        // Constructors
        RenderSystem(Registry& registryRef, Renderer& rendererRef);
        ~RenderSystem() override = default;

        // Functions
        void render(Time dt, RenderLayer layer) override;
        RenderLayer get_render_layers() const override { return RenderLayer::Geometry; }

        // No fields of its own to save, registryRef/rendererRef are construction dependencies
        // (see SystemFactory), not tunable state. Empty so RenderSystem can still be registered
        // via SystemCatalog::register_system<T>() (requires Reflectable<T>).
        DRAFT_REFLECTABLE(RenderSystem)
    };
}
