#include "draft/ecs/render_system.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"

namespace Draft {
    // Constructors
    RenderSystem::RenderSystem(Registry& registryRef, Renderer& rendererRef) : registryRef(registryRef), rendererRef(rendererRef) {}

    // Functions
    void RenderSystem::render(Time, RenderLayer){
        auto view = registryRef.view<SpriteComponent, TransformComponent>();

        for(auto entity : view){
            auto& spriteComponent = view.get<SpriteComponent>(entity);
            auto& transformComponent = view.get<TransformComponent>(entity);

            Material2D mat;
            mat.baseTexture = spriteComponent.texture.texture.get();
            mat.shader = spriteComponent.shader;

            rendererRef.batch.draw({
                transformComponent.position,
                transformComponent.rotation,
                spriteComponent.size,
                spriteComponent.origin,
                spriteComponent.zIndex,
                spriteComponent.texture.bounds,
                mat
            });
        }
    }
}
