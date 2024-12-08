#pragma once

#include "nlohmann/json.hpp" // IWYU pragma: keep

namespace Draft {
    // Would make this just a namespace but I'd like to use friends for more encapsulation
    struct Serializer {
        template<typename T>
        static void serialize(T& obj, std::vector<std::byte>& byteArray);

        template<typename T>
        static void deserialize(T& obj, std::vector<std::byte>& byteArray);

        template<typename T>
        static void serialize(T& obj, nlohmann::json& json);

        template<typename T>
        static void deserialize(T& obj, nlohmann::json& json);
    };
};