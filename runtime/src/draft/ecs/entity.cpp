#include "draft/ecs/entity.hpp"

namespace Draft {
    Entity::Entity(Scene* context, entt::entity entityID)
        : m_entityID(entityID), m_context(context) {}

    bool Entity::is_valid() const {
        return m_context != nullptr && m_entityID != entt::null && m_context->get_registry().valid(m_entityID);
    }

    Scene* Entity::get_scene(){
        return m_context;
    }

    bool Entity::destroy(){
        if(!is_valid())
            return false;

        m_context->get_registry().destroy(m_entityID);
        m_entityID = entt::null;
        m_context = nullptr;

        return true;
    }
}
