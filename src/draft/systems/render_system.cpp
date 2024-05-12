#include "draft/rendering/camera.hpp"
#include "draft/rendering/sprite_batch.hpp"
#include "draft/systems.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"

void Draft::render_system(Registry& registry, SpriteBatch& batch, RenderWindow& window, const Camera* camera){
    auto view = registry.view<SpriteComponent, TransformComponent>();

    for(auto entity : view){
        auto& spriteComponent = view.get<SpriteComponent>(entity);
        auto& transformComponent = view.get<TransformComponent>(entity);

        batch.draw({
            &spriteComponent.texture,
            transformComponent.position,
            transformComponent.rotation,
            spriteComponent.size,
            spriteComponent.origin,
            spriteComponent.zIndex
        });
    }

    batch.flush(window, camera);
}