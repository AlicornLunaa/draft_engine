#include "draft/core/engine.hpp"
#include "draft/components/tag_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/components/texture_component.hpp"
#include "draft/ecs/relationship_components.hpp"

namespace Draft {
    Engine::Engine(){
        register_component<TransformComponent>();
        register_component<TagComponent>();
        register_component<ParentComponent>();
        register_component<ChildComponent>();
        register_component<TextureComponent>();
    }
}
