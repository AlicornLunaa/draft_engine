#include "draft/core/entity.hpp"

namespace Draft {
    Entity::Entity(Scene* context, entt::entity entityID)
        : context(context), entityID(entityID) {}
}
