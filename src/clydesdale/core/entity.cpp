#include "entity.hpp"
using namespace Clydesdale::Core;
Entity::Entity(Scene* context, entt::entity entityID) : context(context), entityID(entityID) {}