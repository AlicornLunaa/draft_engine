#include "draft/core/entity.hpp"

namespace Draft {
    Entity::Entity(Scene* context, entt::entity entityID)
        : context(context), entityID(entityID) {}


    bool Entity::is_valid() const {
        return !(entityID == entt::null || !context);
    }

    bool Entity::destroy(){
        if(entityID == entt::null || !context)
            return false;

        context->get_registry().destroy(entityID);
        entityID = entt::null;
        context = nullptr;
        
        return true;
    }
}
