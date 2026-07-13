#pragma once

#include "draft/util/files/file_handle.hpp"
#include "nlohmann/json.hpp"
#include <string>

namespace Draft {
    /**
     * @brief A drop-in nlohmann::json with extra constructors for this engine's own types.
     *
     * operator[]/at() are overridden to return JSON& instead of the inherited nlohmann::json&, so
     * chained/recursive access never needs a cast back to JSON. This is safe since JSON adds no
     * fields or virtual functions of its own, a reference to the underlying nlohmann::json is
     * always layout-compatible with one to JSON.
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

        JSON& operator[](const std::string& key){ return static_cast<JSON&>(nlohmann::json::operator[](key)); }
        const JSON& operator[](const std::string& key) const { return static_cast<const JSON&>(nlohmann::json::operator[](key)); }
        template<typename T> JSON& operator[](T* key){ return static_cast<JSON&>(nlohmann::json::operator[](key)); }
        template<typename T> const JSON& operator[](T* key) const { return static_cast<const JSON&>(nlohmann::json::operator[](key)); }
        JSON& operator[](size_t index){ return static_cast<JSON&>(nlohmann::json::operator[](index)); }
        const JSON& operator[](size_t index) const { return static_cast<const JSON&>(nlohmann::json::operator[](index)); }

        JSON& at(const std::string& key){ return static_cast<JSON&>(nlohmann::json::at(key)); }
        const JSON& at(const std::string& key) const { return static_cast<const JSON&>(nlohmann::json::at(key)); }
        JSON& at(size_t index){ return static_cast<JSON&>(nlohmann::json::at(index)); }
        const JSON& at(size_t index) const { return static_cast<const JSON&>(nlohmann::json::at(index)); }

        // Static factories, wrapped so callers get a JSON back instead of the base
        // nlohmann::json (which, being an unrelated overload candidate, can make an ambiguous
        // call out of
        template<typename... Args>
        static JSON parse(Args&&... args){ return JSON(nlohmann::json::parse(std::forward<Args>(args)...)); }

        static JSON array(initializer_list_t init = {}){ return JSON(nlohmann::json::array(init)); }
        static JSON object(initializer_list_t init = {}){ return JSON(nlohmann::json::object(init)); }
    };

    /**
     * @brief Lets nlohmann's own machinery convert a JSON into a plain nlohmann::json (assigning
     * or constructing one from the other) without going through generic object serialization.
     * Without this, nlohmann treats JSON as some unrelated serializable type (its SFINAE checks
     * only exclude the exact type nlohmann::json, not JSON), and misreads it as a container.
     */
    inline void to_json(nlohmann::json& j, const JSON& value){
        j = static_cast<const nlohmann::json&>(value);
    }

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
