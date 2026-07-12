#pragma once

#include "draft/util/files/file_handle.hpp"
#include "nlohmann/json.hpp"
#include <string>

namespace Draft {
    /**
     * @brief A drop-in nlohmann::json with extra constructors for this engine's own types.
     */
    class JSON : public nlohmann::json {
    public:
        // Inherit constructors
        using nlohmann::json::json;

        /**
         * @brief Parses @p handle's contents as JSON.
         */
        JSON(FileHandle handle) : nlohmann::json(nlohmann::json::parse(handle.read_string())) {}

        JSON(const nlohmann::json& other) : nlohmann::json(other) {}
        JSON(nlohmann::json&& other) noexcept : nlohmann::json(std::move(other)) {}
    };

    /**
     * @brief Interface for types that can encode/decode themselves to and from a JSON string via
     * runtime polymorphism.
     *
     * This exists alongside Serializer's JsonSerializable concept (see serializer.hpp) rather
     * than replacing it JsonSerializable is for concrete, compile-time-known types (no vtable,
     * dispatched via static functions), while JSONInterface is for cases that need to serialize
     * a heterogeneous collection through a common base pointer/reference.
     */
    class JSONInterface {
    public:
        JSONInterface() = default;
        virtual ~JSONInterface() = default;

        /**
         * @brief Encodes this object as a JSON string.
         */
        virtual std::string encode() const = 0;

        /**
         * @brief Populates this object's state from a JSON string produced by encode().
         */
        virtual void decode(const std::string& data) = 0;
    };

    /**
     * @brief Convenience wrapper for calling JSONInterface::encode() through a reference.
     */
    std::string encode_json(const JSONInterface& data);

    /**
     * @brief Convenience wrapper for calling JSONInterface::decode() through a reference.
     */
    void decode_json(JSONInterface& data, const std::string& str);
};
