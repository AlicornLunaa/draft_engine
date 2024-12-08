#pragma once

#include "nlohmann/json.hpp" // IWYU pragma: keep

// This is used to create the function declarations for specialized serializations
#define DRAFT_CREATE_SERIALIZER(type)                                   \
    namespace Serializer {                                              \
        size_t serialize(type& obj, std::vector<std::byte>& byteArray); \
        size_t deserialize(type& obj, std::byte* bytePtr);              \
        void serialize(type& obj, nlohmann::json& json);                \
        void deserialize(type& obj, nlohmann::json& json);              \
    };

// This is used to expose private data for special serializations of custom classes
#define DRAFT_EXPOSE_SERIALIZER(type)                                                   \
    friend size_t Draft::Serializer::serialize(type& obj, std::vector<std::byte>& byteArray);  \
    friend size_t Draft::Serializer::deserialize(type& obj, std::byte* bytePtr);               \
    friend void Draft::Serializer::serialize(type& obj, nlohmann::json& json);                 \
    friend void Draft::Serializer::deserialize(type& obj, nlohmann::json& json);

// Generic serializer macro for quickly adding normal types. This will only work on simple types with no pointers, virtuals, etc
#define DRAFT_GENERIC_SERIALIZER(type)                                                  \
    size_t Draft::Serializer::serialize(type& obj, std::vector<std::byte>& byteArray){  \
        size_t byteCount = sizeof(obj);                                                 \
        char* bytes = new char[byteCount];                                              \
        std::memcpy(bytes, &obj, byteCount);                                            \
                                                                                        \
        for(size_t i = 0; i < byteCount; i++)                                           \
            byteArray.push_back(static_cast<std::byte>(bytes[i]));                      \
                                                                                        \
        delete[] bytes;                                                                 \
        return byteCount;                                                               \
    }                                                                                   \
                                                                                        \
    size_t Draft::Serializer::deserialize(type& obj, std::byte* bytePtr){               \
        size_t byteCount = sizeof(obj);                                                 \
        std::memcpy(&obj, bytePtr, byteCount);                                          \
        return byteCount;                                                               \
    }                                                                                   \
                                                                                        \
    void Draft::Serializer::serialize(type& obj, nlohmann::json& json){                 \
        json = obj;                                                                     \
    }                                                                                   \
                                                                                        \
    void Draft::Serializer::deserialize(type& obj, nlohmann::json& json){               \
        obj = json.template get<type>();                                                \
    }

// Main declarations here
namespace Draft {
    namespace Serializer {
        // Generic specializations, itll work for basic shallow data types. Anything complex, using the heap, etc, has to be defined in the source file
        // template<typename T>
        // size_t serialize(T& obj, std::vector<std::byte>& byteArray){
        //     // Save this type into the array
        //     size_t byteCount = sizeof(obj);
        //     char* bytes = new char[byteCount];
        //     std::memcpy(bytes, &obj, byteCount);

        //     for(size_t i = 0; i < byteCount; i++)
        //         byteArray.push_back(static_cast<std::byte>(bytes[i]));

        //     delete[] bytes;
        //     return byteCount;
        // }

        // template<typename T>
        // size_t deserialize(T& obj, std::byte* bytePtr){
        //     // Load each type
        //     size_t byteCount = sizeof(obj);
        //     std::memcpy(&obj, bytePtr, byteCount);
        //     return byteCount;
        // }

        // template<typename T>
        // void serialize(T& obj, nlohmann::json& json){
        //     // Save this object to json
        //     json = obj;
        // }

        // template<typename T>
        // void deserialize(T& obj, nlohmann::json& json){
        //     // Load this from json type
        //     obj = json.template get<T>();
        // }
    };

    DRAFT_CREATE_SERIALIZER(int);
    DRAFT_CREATE_SERIALIZER(uint);
    DRAFT_CREATE_SERIALIZER(float);
};