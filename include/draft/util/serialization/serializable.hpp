#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "draft/util/serialization/serializer.hpp"
#include "draft/util/serialization/string_serializer.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep

#define SerializeField(var) \
    Draft::Serializable::fieldMap.emplace(#var, var);

namespace Draft {
    // Define specialized serialization functions for the serializer class
    class Serializable;
    DRAFT_CREATE_SERIALIZER(Serializable);

    // Interface to implement for a class to be serializable. Handles everything with one inheitance and a macro
    class Serializable {
    protected:
        // Types
        typedef std::vector<std::byte> ByteArray;
        typedef nlohmann::json Json;

        struct Field {
            void* ptr = nullptr;

            std::function<size_t(ByteArray& bytes)> encodeBytes; // Turns the ptr into bytes. Returns the length in bytes
            std::function<size_t(std::byte* bytePtr)> decodeBytes; // Turns the bytes into ptr. Returns the length in bytes
            std::function<void(Json& json)> encodeJSON; // Turns the ptr into json
            std::function<void(Json& json)> decodeJSON; // Turns the json into ptr

            template<typename T>
            Field(T& obj) : ptr(&obj) {
                // Bytes
                encodeBytes = [this](ByteArray& bytes){ return Serializer::serialize(*static_cast<T*>(ptr), bytes); };
                decodeBytes = [this](std::byte* bytePtr){ return Serializer::deserialize(*static_cast<T*>(ptr), bytePtr); };
                
                // JSON
                encodeJSON = [this](Json& json){ Serializer::serialize(*static_cast<T*>(ptr), json); };
                decodeJSON = [this](Json& json){ Serializer::deserialize(*static_cast<T*>(ptr), json); };
            }
        };

        // Variables
        std::unordered_map<std::string, Field> fieldMap; // This pair is the ptr to the data and the length in bytes

    public:
        // Constructors
        Serializable() = default;
        virtual ~Serializable() = default;

        // Friends
        DRAFT_EXPOSE_SERIALIZER(Serializable);
    };
};