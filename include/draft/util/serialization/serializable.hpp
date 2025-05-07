#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "draft/util/serialization/serializer.hpp"
#include "draft/util/serialization/string_serializer.hpp"
#include "draft/util/json.hpp"

#define SerializeField(var) \
    Draft::Serializable::fieldMap.emplace(#var, var);

namespace Draft {
    // Define specialized serialization functions for the serializer class
    class Serializable;
    DRAFT_CREATE_SERIALIZER(Serializable);

    // Interface to implement for a class to be serializable. Handles everything with one inheitance and a macro
    // Every move/copy operator/constructor is empty because fields should be defined for every class that inherits
    class Serializable {
    protected:
        // Types
        typedef std::vector<std::byte> ByteArray;

        struct Field {
            void* ptr = nullptr;

            std::function<size_t(ByteArray& bytes)> encodeBytes; // Turns the ptr into bytes. Returns the length in bytes
            std::function<size_t(std::byte* bytePtr)> decodeBytes; // Turns the bytes into ptr. Returns the length in bytes
            std::function<void(JSON& json)> encodeJSON; // Turns the ptr into json
            std::function<void(JSON& json)> decodeJSON; // Turns the json into ptr

            template<typename T>
            Field(T& obj) : ptr(&obj) {
                // Bytes
                encodeBytes = [this](ByteArray& bytes){ return Serializer::serialize(*static_cast<T*>(ptr), bytes); };
                decodeBytes = [this](std::byte* bytePtr){ return Serializer::deserialize(*static_cast<T*>(ptr), bytePtr); };
                
                // JSON
                encodeJSON = [this](JSON& json){ Serializer::serialize(*static_cast<T*>(ptr), json); };
                decodeJSON = [this](JSON& json){ Serializer::deserialize(*static_cast<T*>(ptr), json); };
            }
        };

        // Variables
        std::unordered_map<std::string, Field> fieldMap; // This pair is the ptr to the data and the length in bytes

    public:
        // Constructors
        Serializable() = default;
        Serializable(const Serializable& other){};
        Serializable(Serializable&& other){};
        virtual ~Serializable() = default;

        // Operators
        Serializable& operator=(const Serializable& other){ return *this; };
        Serializable& operator=(Serializable&& other){ return *this; };

        // Friends
        DRAFT_EXPOSE_SERIALIZER(Serializable);
    };
};