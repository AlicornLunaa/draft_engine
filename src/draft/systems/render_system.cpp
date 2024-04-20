#include "draft/rendering/camera.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/sprite_batch.hpp"
#include "draft/systems.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"

void Draft::render_system(Registry& registry, RenderWindow& window, Shader& shader, const Camera& camera){
    auto view = registry.view<SpriteComponent, TransformComponent>();
    SpriteBatch batch{};

    shader.bind();
    shader.set_uniform("view", camera.get_view());
    shader.set_uniform("projection", camera.get_projection());
    
    for(auto entity : view){
        auto& spriteComponent = view.get<SpriteComponent>(entity);
        auto& transformComponent = view.get<TransformComponent>(entity);

        batch.draw(
            spriteComponent.texture,
            transformComponent.position,
            spriteComponent.size,
            transformComponent.rotation,
            spriteComponent.origin
        );
    }

    batch.flush();
    shader.unbind();
}