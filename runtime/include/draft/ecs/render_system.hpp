#pragma once

#include "draft/ecs/registry.hpp"
#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class ApplicationInterface;

    /**
     * @brief Submits every <SpriteComponent, TransformComponent> entity into the owning
     * ApplicationInterface's current Renderer's sprite batch every frame. Handles
     * AnimationComponent overrides for SpriteComponent
     */
    class RenderSystem : public AbstractSystem {
    private:
        // Variables
        ApplicationInterface& appRef;
        Registry& registryRef;

    public:
        // Constructors
        RenderSystem(Registry& registryRef, ApplicationInterface& appRef);
        ~RenderSystem() override = default;

        // Functions
        void render(Time dt, RenderLayer layer) override;
        RenderLayer get_render_layers() const override { return RenderLayer::Geometry; }
        
        DRAFT_REFLECTABLE(RenderSystem)
    };
}
