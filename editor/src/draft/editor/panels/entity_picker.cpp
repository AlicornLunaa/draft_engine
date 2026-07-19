#include "draft/editor/panels/entity_picker.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"

#include <limits>

namespace Draft {
    Entity pick_entity(Scene& scene, const Vector2f& worldPoint){
        Registry& registry = scene.get_registry();

        Entity best;
        float bestZIndex = -std::numeric_limits<float>::infinity();

        for(auto [raw, transform, sprite] : registry.view<TransformComponent, SpriteComponent>().each()){
            Vector2f min = transform.position - sprite.origin;
            Vector2f max = min + sprite.size;

            bool inside = worldPoint.x >= min.x && worldPoint.x <= max.x
                && worldPoint.y >= min.y && worldPoint.y <= max.y;

            if(inside && sprite.zIndex >= bestZIndex){
                bestZIndex = sprite.zIndex;
                best = Entity(&scene, raw);
            }
        }

        if(best.is_valid())
            return best;

        // No sprite matched, fall back to shape-accurate testing against ColliderComponent.
        for(auto [raw, transform, colliderComp] : registry.view<TransformComponent, ColliderComponent>().each()){
            Vector2f localPoint = Math::inverse(transform.get_transform()) * Vector3f(worldPoint, 1.f);
            Collider& collider = colliderComp.collider;

            if(collider.test_point(localPoint))
                return Entity(&scene, raw);
        }

        return Entity();
    }
}
