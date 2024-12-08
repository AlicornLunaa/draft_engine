#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>

#include "nlohmann/json.hpp" // IWYU pragma: keep

#define SerializeField(var) \
    Serializable::fieldMap[#var] = {static_cast<void*>(&var), sizeof(var)};

namespace Draft {
    class Serializable {
    protected:
        // Variables
        std::unordered_map<std::string, std::pair<void*, size_t>> fieldMap; // This pair is the ptr to the data and the length in bytes

    public:
        // Constructors
        Serializable() = default;
        virtual ~Serializable() = default;

        // Friends
        friend class Serializer;
    };
};