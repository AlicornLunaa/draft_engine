#include "draft/core/entity.hpp"

namespace Draft {
    Entity::Entity(Scene* context, entt::entity entityID)
        : m_context(context), m_entityID(entityID) {}


    bool Entity::is_valid() const {
        return !(m_entityID == entt::null || !m_context);
    }

    Scene* Entity::get_scene(){
        return m_context;
    }

    bool Entity::destroy(){
        if(m_entityID == entt::null || !m_context)
            return false;

        m_context->get_registry().destroy(m_entityID);
        m_entityID = entt::null;
        m_context = nullptr;
        
        return true;
    }
}
