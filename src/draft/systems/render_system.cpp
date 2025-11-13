#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"
#include <tracy/Tracy.hpp>
#include "draft/systems/render_system.hpp"

namespace Draft {
    // Constructors
    RenderSystem::RenderSystem(Registry& registryRef, RenderWindow& windowRef) : registryRef(registryRef), windowRef(windowRef) {
        // Attach listeners
    }

    RenderSystem::~RenderSystem(){
    }

    // Functions
    void RenderSystem::render(SpriteBatch& batch){
        ZoneScopedN("render_system");

        auto view = registryRef.view<SpriteComponent, TransformComponent>();

        for(auto entity : view){
            auto& spriteComponent = view.get<SpriteComponent>(entity);
            auto& transformComponent = view.get<TransformComponent>(entity);

            Material2D mat;
            mat.baseTexture = spriteComponent.texture.texture;

            batch.draw({
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
};