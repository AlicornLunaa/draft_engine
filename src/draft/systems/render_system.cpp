#include "draft/systems.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/texture_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/math/vector2_p.hpp"

void Draft::renderSystem(entt::registry& registry, sf::RenderWindow* window){
    auto view = registry.view<SpriteComponent, TextureComponent, TransformComponent>();
    sf::RectangleShape spriteShape;
    
    for(auto entity : view){
        auto& spriteComponent = view.get<SpriteComponent>(entity);
        auto& textureComponent = view.get<TextureComponent>(entity);
        auto& transformComponent = view.get<TransformComponent>(entity);

        sf::RenderStates states = sf::RenderStates::Default;
        states.shader = spriteComponent.shader;
        states.texture = textureComponent;
        states.transform = transformComponent;

        spriteShape.setTexture(textureComponent);
        spriteShape.setPosition(vector_to_sf(spriteComponent.position));
        spriteShape.setRotation(spriteComponent.rotation);
        spriteShape.setSize(vector_to_sf(spriteComponent.size));

        window->draw(spriteShape, states);
    }
}