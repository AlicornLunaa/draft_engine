#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/util/serialization/context.hpp"
#include "draft/util/serialization/serializer.hpp"

#include <cstdint>

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

    void Entity::serialize(const Entity& entity, Binary::ByteArray& out){
        uint32_t id = entity == NULL_ENTITY ? UINT32_MAX : Serializer::context<SceneSerializationContext>().entityToId.at(entity.m_entityID);
        Serializer::serialize(id, out);
    }

    void Entity::deserialize(Entity& entity, Binary::ByteView span){
        uint32_t id;
        Serializer::deserialize(id, span);

        entity = id == UINT32_MAX ? NULL_ENTITY : Serializer::context<SceneSerializationContext>().idToEntity.at(id);
    }

    void Entity::serialize(const Entity& entity, JSON& json){
        uint32_t id = entity == NULL_ENTITY ? UINT32_MAX : Serializer::context<SceneSerializationContext>().entityToId.at(entity.m_entityID);
        Serializer::serialize(id, json);
    }

    void Entity::deserialize(Entity& entity, const JSON& json){
        uint32_t id;
        Serializer::deserialize(id, json);

        entity = id == UINT32_MAX ? NULL_ENTITY : Serializer::context<SceneSerializationContext>().idToEntity.at(id);
    }
}
