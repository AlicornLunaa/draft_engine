#pragma once

#include "draft/util/file_handle.hpp"
#include "nlohmann/json.hpp"
#include <string>

namespace Draft {
    // Types
    class JSON : public nlohmann::json {
    public:
        // Inherit constructors
        using nlohmann::json::json;

        // Custom constructor
        JSON(FileHandle handle) : nlohmann::json(nlohmann::json::parse(handle.read_string())) {}
        JSON(const nlohmann::json& other) : nlohmann::json(other) {}
        JSON(nlohmann::json&& other) noexcept : nlohmann::json(std::move(other)) {}
    };

    // Interface
    class JSONInterface {
    public:
        // Constructors
        JSONInterface();
        virtual ~JSONInterface();

        // Functions
        virtual std::string encode() const = 0;
        virtual void* decode(const std::string& data) = 0;
    };

    // Functions
    std::string encode_json(const JSONInterface& data);
    void* decode_json(const std::string& data);
};