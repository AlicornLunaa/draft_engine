#include "entity.hpp"

namespace Clydesdale {
    Entity::Entity(Scene* context, entt::entity entityID)
        : context(context), entityID(entityID) {}
}
