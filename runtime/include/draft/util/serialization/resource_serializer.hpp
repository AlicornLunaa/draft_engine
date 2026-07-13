#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/asset/resource.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/util/serialization/context.hpp"
#include "draft/util/serialization/serializer.hpp"

#include <string>

/**
 * @brief Out-of-line definitions for the Resource<T>::serialize/deserialize declared in
 * resource.hpp. An unresolvable resource (invalid, or not currently reachable by key, e.g. a placeholder) writes
 * as an empty string and reads back as a default-constructed (invalid) Resource<T>.
 */
namespace Draft {
    template<typename T>
    void Resource<T>::serialize(const Resource<T>& resource, Binary::ByteArray& out){
        std::string key = Serializer::context<SceneSerializationContext>().assets->key_for<T>(resource).value_or("");
        Serializer::serialize(key, out);
    }

    template<typename T>
    void Resource<T>::deserialize(Resource<T>& resource, Binary::ByteView span){
        std::string key;
        Serializer::deserialize(key, span);
        resource = key.empty() ? Resource<T>() : Serializer::context<SceneSerializationContext>().assets->get<T>(key);
    }

    template<typename T>
    void Resource<T>::deserialize_and_advance(Resource<T>& resource, Binary::ByteView& span){
        std::string key;
        Serializer::deserialize_and_advance(key, span);
        resource = key.empty() ? Resource<T>() : Serializer::context<SceneSerializationContext>().assets->get<T>(key);
    }

    template<typename T>
    void Resource<T>::serialize(const Resource<T>& resource, JSON& json){
        std::string key = Serializer::context<SceneSerializationContext>().assets->key_for<T>(resource).value_or("");
        Serializer::serialize(key, json);
    }

    template<typename T>
    void Resource<T>::deserialize(Resource<T>& resource, const JSON& json){
        std::string key;
        Serializer::deserialize(key, json);
        resource = key.empty() ? Resource<T>() : Serializer::context<SceneSerializationContext>().assets->get<T>(key);
    }
}
