#include "draft/ecs/render_system.hpp"
#include "draft/components/animation_component.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"

namespace Draft {
    // Constructors
    RenderSystem::RenderSystem(Registry& registryRef, Renderer& rendererRef) : registryRef(registryRef), rendererRef(rendererRef) {}

    // Functions
    void RenderSystem::render(Time dt, RenderLayer){
        // Setting the texture for the sprite via an animation
        for(auto&& [entity, sprite, animation] : registryRef.view<SpriteComponent, AnimationComponent>().each()){
            if(!animation.animation || animation.animation->get_frames().empty())
                continue;

            if(animation.tag.empty()){
                sprite.texture = animation.animation->get_frame(animation.frameTime);
            } else {
                sprite.texture = animation.animation->get_frame(animation.tag, animation.frameTime);
            }

            animation.frameTime += dt.as_milliseconds();
        }

        // Actually rendering the sprite
        for(const auto& [entity, spriteComponent, transformComponent] : registryRef.view<SpriteComponent, TransformComponent>().each()){
            Material2D mat;
            mat.baseTexture = spriteComponent.texture.texture.get();
            mat.shader = spriteComponent.shader ? spriteComponent.shader->get() : nullptr;

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
