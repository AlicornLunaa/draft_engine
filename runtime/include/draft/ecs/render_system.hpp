#pragma once

#include "draft/ecs/registry.hpp"
#include "draft/ecs/system.hpp"
#include "draft/rendering/pipeline/renderer.hpp"

namespace Draft {
    /**
     * @brief Submits every <SpriteComponent, TransformComponent> entity into the owning
     * Renderer's sprite batch every frame.
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
    };
}
