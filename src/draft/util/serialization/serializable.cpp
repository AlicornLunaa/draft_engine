#include "draft/util/serialization/serializable.hpp"
#include <cstddef>
#include <cstring>

// When serializing check if the type member type can be serialized
// maybe do this with a map of type_index with pointers to function definitions?

namespace Draft {
    // Serializable type
    size_t Serializer::serialize(Serializable& obj, std::vector<std::byte>& byteArray){
        // Save each type
        size_t byteCount = 0;

        for(auto& [key, field] : obj.fieldMap){
            byteCount += field.encodeBytes(byteArray);
        }

        return byteCount;
    }

    size_t Serializer::deserialize(Serializable& obj, std::byte* bytePtr){
        // Load each type
        size_t byteCount = 0;

        for(auto& [key, field] : obj.fieldMap){
            byteCount += field.decodeBytes(bytePtr + byteCount);
        }

        return byteCount;
    }
    
    void Serializer::serialize(Serializable& obj, JSON& json){
        // Save each type
        for(auto& [key, field] : obj.fieldMap){
            JSON temp;
            field.encodeJSON(temp);
            json[key] = temp;
        }
    }

    void Serializer::deserialize(Serializable& obj, JSON& json){
        // Load each type
        for(auto& [key, field] : obj.fieldMap){
            JSON temp = json[key];
            field.decodeJSON(temp);
        }
    }
};