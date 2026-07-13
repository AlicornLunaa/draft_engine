#pragma once

namespace Draft {
    namespace Serializer {
        /**
         * @brief Customization point for giving a type explicit (de)serialization without adding
         * static serialize/deserialize members to the type itself, for types you don't own (e.g.
         * glm::vec2) or just don't want to touch. Specialize this template for T in your own
         * header; nothing here needs to change or know about it ahead of time. Left undefined on
         * purpose as it is only ever named inside unevaluated requires-expressions or specialized, never
         * instantiated as-is.
         *
         * @code
         * // in your own header, e.g. glm_serializer.hpp
         * template<>
         * struct Draft::Serializer::CustomSerializer<glm::vec2> {
         *     static void serialize(const glm::vec2& v, JSON& json) { json = {v.x, v.y}; }
         *     static void deserialize(glm::vec2& v, JSON& json) { v = {json[0], json[1]}; }
         *     // ...and/or the Binary::ByteArray&/Binary::ByteView overloads, independently -
         *     // Binary and JSON support don't both need to be specialized.
         * };
         * @endcode
         */
        template<typename T>
        struct CustomSerializer;
    }
}
