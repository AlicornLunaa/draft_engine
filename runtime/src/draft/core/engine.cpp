#include "draft/core/engine.hpp"
#include "draft/components/animation_component.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/components/joint_component.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/tag_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/texture_component.hpp"
#include "draft/ecs/relationship_components.hpp"
#include "draft/util/serialization/resource_serializer.hpp" // IWYU pragma: keep

namespace Draft {
    Engine::Engine(){
        register_component<TransformComponent>();
        register_component<TagComponent>();
        register_component<ParentComponent>();
        register_component<ChildComponent>();
        register_component<TextureComponent>();
        register_component<SpriteComponent>();
        register_component<ColliderComponent>();
        register_component<ConstrainedComponent>();
        register_component<RigidBodyComponent>();
        register_component<AnimationComponent>();
    }
}
