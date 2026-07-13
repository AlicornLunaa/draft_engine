#pragma once

#include "draft/ecs/entity.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Draft {
    class AssetManager;

    /**
     * @brief The Ctx used with Serializer::ScopedContext<SceneSerializationContext> for the
     * duration of a save_scene()/load_scene() call. Entity/Resource<T> reach into this (via
     * Serializer::context<SceneSerializationContext>()) to resolve the external state their
     * tier-1 (de)serialization needs that doesn't fit Serializer's fixed 2-arg signature.
     */
    struct SceneSerializationContext {
        AssetManager* assets;

        // Save direction: live entt::entity -> sequential file id.
        std::unordered_map<entt::entity, uint32_t> entityToId;

        // Load direction: sequential file id -> newly created Entity.
        std::vector<Entity> idToEntity;
    };
}
