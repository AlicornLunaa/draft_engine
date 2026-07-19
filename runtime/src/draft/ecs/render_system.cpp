#include "draft/ecs/render_system.hpp"
#include "draft/components/animation_component.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"

namespace Draft {
    // Constructors
    RenderSystem::RenderSystem(Registry& registryRef, Renderer& rendererRef) : registryRef(registryRef), rendererRef(rendererRef) {}

    // Functions
    void RenderSystem::render(Time dt, RenderLayer){
        for(const auto& [entity, spriteComponent, transformComponent] : registryRef.view<SpriteComponent, TransformComponent>().each()){
            TextureRegion region = spriteComponent.texture;

            if(auto* animComp = registryRef.try_get<AnimationComponent>(entity)){
                // Animation component exists, it should take precedence over the sprite
                if(animComp->animation && !animComp->animation->get_frames().empty() && (animComp->animation->has_tag(animComp->tag) || animComp->tag.empty())){
                    if(animComp->tag.empty()){
                        region = animComp->animation->get_frame(animComp->frameTime);
                    } else {
                        region = animComp->animation->get_frame(animComp->tag, animComp->frameTime);
                    }

                    animComp->frameTime += dt.as_milliseconds();
                }
            }

            Material2D mat;
            mat.baseTexture = region.texture.get();
            mat.shader = spriteComponent.shader ? spriteComponent.shader->get() : nullptr;

            rendererRef.batch.draw({
                transformComponent.position,
                transformComponent.rotation,
                spriteComponent.size,
                spriteComponent.origin,
                spriteComponent.zIndex,
                region.bounds,
                mat
            });
        }
    }
}
