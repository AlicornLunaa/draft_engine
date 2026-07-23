#pragma once

#include "draft/util/json.hpp"
#include "draft/util/reflectable.hpp"
#include "draft/util/serialization/base64.hpp"
#include "draft/util/serialization/custom.hpp"
#include "draft/util/serialization/binary.hpp"

#include <concepts>
#include <string>
#include <type_traits>

namespace Draft {
    /**
     * @brief Type-driven serialization on top of Binary and Draft::JSON.
     *
     * serialize()/deserialize() are overloaded (not specialized) so a type opts in just by
     * satisfying one of the concepts below (BinarySerializable/JsonSerializable) with its own
     * static serialize/deserialize functions, or by being one of the built-in trivial/vector
     * cases already handled here.
     */
    namespace Serializer {
        /**
         * @brief Satisfied by types with their own static Binary (de)serialization functions.
         */
        template<typename T>
        concept BinarySerializable = requires(T object, Binary::ByteArray& buffer, Binary::ByteView& span){
            { T::serialize(object, buffer) } -> std::convertible_to<void>;
            { T::deserialize(object, span) } -> std::convertible_to<void>;
        };

        /**
         * @brief Satisfied by types with their own static JSON (de)serialization functions.
         * deserialize() only reads from its JSON, so it takes it by const reference.
         */
        template<typename T>
        concept JsonSerializable = requires(T object, Draft::JSON& json, const Draft::JSON& constJson){
            { T::serialize(object, json) } -> std::convertible_to<void>;
            { T::deserialize(object, constJson) } -> std::convertible_to<void>;
        };

        /**
         * @brief Satisfied when a BinarySerializable T also supplies its own
         * deserialize_and_advance, needed for a variable-length T::serialize encoding where the default "consumed exactly sizeof(T)
         * bytes" assumption used for fixed-size explicit types would be wrong.
         */
        template<typename T>
        concept BinarySerializableWithAdvance = BinarySerializable<T> && requires(T object, Binary::ByteView& span){
            { T::deserialize_and_advance(object, span) } -> std::convertible_to<void>;
        };

        /**
         * @brief Satisfied once CustomSerializer<T> has been specialized with Binary methods.
         */
        template<typename T>
        concept CustomBinarySerializable = requires(T object, Binary::ByteArray& buffer, Binary::ByteView& span){
            { CustomSerializer<T>::serialize(object, buffer) } -> std::convertible_to<void>;
            { CustomSerializer<T>::deserialize(object, span) } -> std::convertible_to<void>;
        };

        /**
         * @brief Satisfied once CustomSerializer<T> has been specialized with JSON methods.
         */
        template<typename T>
        concept CustomJsonSerializable = requires(T object, Draft::JSON& json, const Draft::JSON& constJson){
            { CustomSerializer<T>::serialize(object, json) } -> std::convertible_to<void>;
            { CustomSerializer<T>::deserialize(object, constJson) } -> std::convertible_to<void>;
        };

        /**
         * @brief Satisfied when a CustomBinarySerializable T's CustomSerializer also supplies its
         * own deserialize_and_advance, needed for variable-length types (e.g. a string or
         * vector), where the default "consumed exactly sizeof(T) bytes" assumption used for
         * fixed-size custom types would be wrong.
         */
        template<typename T>
        concept CustomBinarySerializableWithAdvance = CustomBinarySerializable<T> && requires(T object, Binary::ByteView& span){
            { CustomSerializer<T>::deserialize_and_advance(object, span) } -> std::convertible_to<void>;
        };

        /**
         * @brief Satisfied by trivially copyable types with no explicit Binary (de)serialization
         * of their own, the lowest-priority Binary tier.
         */
        template<typename T>
        concept TriviallySerializable = std::is_trivially_copyable_v<T>;

        /**
         * @brief Satisfied by arithmetic/enum/std::string types with no explicit JSON
         * (de)serialization of their own, the lowest-priority JSON tier.
         */
        template<typename T>
        concept JsonTrivial = std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string>;

        /**
         * @brief Satisfied only by JSON itself, in any value category, not by anything merely
         * convertible to it. The JSON tiers below deduce their json parameter through this rather
         * than taking a plain JSON&/const JSON&
         */
        template<typename J>
        concept JsonLike = std::same_as<std::remove_cvref_t<J>, JSON>;

        /**
         * @brief Satisfied by types with some JSON (de)serialization path (explicit, custom, or
         * trivial) but no Binary path of their own, the lowest-priority Binary tier. Always correct
         * but not compact.
         */
        template<typename T>
        concept JsonBinaryFallback = (JsonSerializable<T> || CustomJsonSerializable<T> || JsonTrivial<T>)
            && !BinarySerializable<T> && !CustomBinarySerializable<T> && !Reflectable<T> && !TriviallySerializable<T>;

        /**
         * @brief Satisfied by types with some Binary (de)serialization path (explicit, custom, or trivial)
         */
        template<typename T>
        concept BinaryJsonFallback = (BinarySerializable<T> || CustomBinarySerializable<T> || TriviallySerializable<T>)
            && !JsonSerializable<T> && !CustomJsonSerializable<T> && !Reflectable<T> && !JsonTrivial<T>;

        /**
         * @name Forward declarations
         * The tiers below are mutually recursive, a reflectable type's fields, or a vector's
         * elements, can themselves be trivial, reflectable, vector, or explicitly-serializable.
         * Every overload is declared here first and defined further down in whatever order.
         * Without this, two-phase lookup wouldn't find a tier defined later in the file from
         * inside an earlier one (e.g. the reflect tier calling into the trivial tier).
         */
        ///@{
        // Explicit tier: member functions on T
        template<BinarySerializable T> void serialize(const T& value, Binary::ByteArray& out);
        template<BinarySerializable T> void deserialize(T& value, Binary::ByteView span);
        template<BinarySerializableWithAdvance T> void deserialize_and_advance(T& value, Binary::ByteView& span);
        template<typename T> requires BinarySerializable<T> && (!BinarySerializableWithAdvance<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);

        template<JsonSerializable T, JsonLike J> void serialize(const T& value, J&& json);
        template<JsonSerializable T, JsonLike J> void deserialize(T& value, J&& json);

        // Explicit tier: out-of-line via a CustomSerializer<T> specialization
        template<typename T> requires CustomBinarySerializable<T> && (!BinarySerializable<T>)
        void serialize(const T& value, Binary::ByteArray& out);
        template<typename T> requires CustomBinarySerializable<T> && (!BinarySerializable<T>)
        void deserialize(T& value, Binary::ByteView span);
        template<typename T> requires CustomBinarySerializableWithAdvance<T> && (!BinarySerializable<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);
        template<typename T> requires CustomBinarySerializable<T> && (!CustomBinarySerializableWithAdvance<T>) && (!BinarySerializable<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);

        template<typename T, JsonLike J> requires CustomJsonSerializable<T> && (!JsonSerializable<T>)
        void serialize(const T& value, J&& json);
        template<typename T, JsonLike J> requires CustomJsonSerializable<T> && (!JsonSerializable<T>)
        void deserialize(T& value, J&& json);

        // Reflectable tier
        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        void serialize(const T& value, Binary::ByteArray& out);
        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        void deserialize(T& value, Binary::ByteView span);
        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);

        template<typename T, JsonLike J> requires Reflectable<T> && (!JsonSerializable<T>) && (!CustomJsonSerializable<T>)
        void serialize(const T& value, J&& json);
        template<typename T, JsonLike J> requires Reflectable<T> && (!JsonSerializable<T>) && (!CustomJsonSerializable<T>)
        void deserialize(T& value, J&& json);

        // Trivial tier
        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        void serialize(const T& value, Binary::ByteArray& out);
        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        void deserialize(T& value, Binary::ByteView span);
        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);

        template<JsonTrivial T, JsonLike J> requires (!JsonSerializable<T>) && (!CustomJsonSerializable<T>) && (!Reflectable<T>)
        void serialize(const T& value, J&& json);
        template<JsonTrivial T, JsonLike J> requires (!JsonSerializable<T>) && (!CustomJsonSerializable<T>) && (!Reflectable<T>)
        void deserialize(T& value, J&& json);

        // JSON-as-bytes fallback tier: JSON-only types with no Binary path of their own
        template<JsonBinaryFallback T> void serialize(const T& value, Binary::ByteArray& out);
        template<JsonBinaryFallback T> void deserialize(T& value, Binary::ByteView span);
        template<JsonBinaryFallback T> void deserialize_and_advance(T& value, Binary::ByteView& span);

        // Binary-as-base64 fallback tier: Binary-only types with no JSON path of their own
        template<BinaryJsonFallback T, JsonLike J> void serialize(const T& value, J&& json);
        template<BinaryJsonFallback T, JsonLike J> void deserialize(T& value, J&& json);
        ///@}

        /**
         * @brief Satisfied by any T that Serializer::serialize()/deserialize() can actually
         * handle end to end, through whichever tier applies (explicit T::serialize,
         * CustomSerializer<T>, Reflectable, or trivial)
         */
        template<typename T>
        concept Serializable = requires(T object, Binary::ByteArray& buffer, Binary::ByteView& span, Draft::JSON& json, const Draft::JSON& constJson){
            { serialize(object, buffer) } -> std::convertible_to<void>;
            { deserialize(object, span) } -> std::convertible_to<void>;
            { serialize(object, json) } -> std::convertible_to<void>;
            { deserialize(object, constJson) } -> std::convertible_to<void>;
        };

        // Default for complex types with explicit binary functions
        template<BinarySerializable T>
        inline void serialize(const T& value, Binary::ByteArray& out){ T::serialize(value, out); }

        template<BinarySerializable T>
        inline void deserialize(T& value, Binary::ByteView span){ T::deserialize(value, span); }

        /**
         * @brief Prefers T's own deserialize_and_advance when it has one, for a variable-length
         * T::serialize encoding (e.g. Resource<T>) where "consumed exactly sizeof(T) bytes"
         * would be wrong.
         */
        template<BinarySerializableWithAdvance T>
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){ T::deserialize_and_advance(value, span); }

        /**
         * @brief Fallback for fixed-size explicit types: assumes T::deserialize consumed exactly
         * sizeof(T) bytes. A variable-length type needs to give itself its own
         * deserialize_and_advance instead (see BinarySerializableWithAdvance above).
         */
        template<typename T> requires BinarySerializable<T> && (!BinarySerializableWithAdvance<T>)
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){
            T::deserialize(value, span);
            span = span.subspan(sizeof(T));
        }

        // Default for explicit JSON-serializable complex types
        template<JsonSerializable T, JsonLike J>
        inline void serialize(const T& value, J&& json){ T::serialize(value, json); }

        template<JsonSerializable T, JsonLike J>
        inline void deserialize(T& value, J&& json){ T::deserialize(value, json); }


        // Default for types with a CustomSerializer<T> specialization
        template<typename T> requires CustomBinarySerializable<T> && (!BinarySerializable<T>)
        inline void serialize(const T& value, Binary::ByteArray& out){ CustomSerializer<T>::serialize(value, out); }

        template<typename T> requires CustomBinarySerializable<T> && (!BinarySerializable<T>)
        inline void deserialize(T& value, Binary::ByteView span){ CustomSerializer<T>::deserialize(value, span); }

        /**
         * @brief Prefers CustomSerializer<T>'s own deserialize_and_advance when it has one, for
         * variable-length custom types (e.g. a string or vector) where "consumed exactly
         * sizeof(T) bytes" would be wrong.
         */
        template<typename T> requires CustomBinarySerializableWithAdvance<T> && (!BinarySerializable<T>)
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){
            CustomSerializer<T>::deserialize_and_advance(value, span);
        }

        /**
         * @brief Fallback for fixed-size custom types: assumes CustomSerializer<T>::deserialize
         * consumed exactly sizeof(T) bytes, same caveat as the BinarySerializable tier's
         * deserialize_and_advance(). Types with a variable-length encoding need to give their
         * CustomSerializer its own deserialize_and_advance instead (see the concept above).
         */
        template<typename T> requires CustomBinarySerializable<T> && (!CustomBinarySerializableWithAdvance<T>) && (!BinarySerializable<T>)
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){
            CustomSerializer<T>::deserialize(value, span);
            span = span.subspan(sizeof(T));
        }

        template<typename T, JsonLike J> requires CustomJsonSerializable<T> && (!JsonSerializable<T>)
        inline void serialize(const T& value, J&& json){ CustomSerializer<T>::serialize(value, json); }

        template<typename T, JsonLike J> requires CustomJsonSerializable<T> && (!JsonSerializable<T>)
        inline void deserialize(T& value, J&& json){ CustomSerializer<T>::deserialize(value, json); }


        // Default for reflectable types (wins over trivial, loses to explicit methods above)
        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        inline void serialize(const T& value, Binary::ByteArray& out){
            for_each_field(value, [&](std::string_view, const auto& field){ serialize(field, out); });
        }

        /**
         * @brief Reflectable types have a variable-length binary encoding in general (any field
         * may itself be variable-length, e.g. a vector), so unlike the trivial tier this can't
         * assume sizeof(T) bytes were consumed. It always defers to deserialize_and_advance().
         */
        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        inline void deserialize(T& value, Binary::ByteView span){
            deserialize_and_advance(value, span);
        }

        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){
            for_each_field(value, [&](std::string_view, auto& field){ deserialize_and_advance(field, span); });
        }

        // Default for reflectable JSON types
        template<typename T, JsonLike J> requires Reflectable<T> && (!JsonSerializable<T>) && (!CustomJsonSerializable<T>)
        inline void serialize(const T& value, J&& json){
            for_each_field(value, [&](std::string_view name, const auto& field){
                serialize(field, json[std::string(name)]);
            });
        }

        template<typename T, JsonLike J> requires Reflectable<T> && (!JsonSerializable<T>) && (!CustomJsonSerializable<T>)
        inline void deserialize(T& value, J&& json){
            for_each_field(value, [&](std::string_view name, auto& field){
                deserialize(field, json.at(std::string(name)));
            });
        }


        // Default for trivial types
        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        inline void serialize(const T& value, Binary::ByteArray& out){ Binary::write(out, value); }

        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        inline void deserialize(T& value, Binary::ByteView span){ Binary::read(span, value); }

        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){
            deserialize(value, span);
            span = span.subspan(sizeof(T));
        }

        // Default for JSON-serializable trivials
        template<JsonTrivial T, JsonLike J> requires (!JsonSerializable<T>) && (!CustomJsonSerializable<T>) && (!Reflectable<T>)
        inline void serialize(const T& value, J&& json){ json = value; }

        template<JsonTrivial T, JsonLike J> requires (!JsonSerializable<T>) && (!CustomJsonSerializable<T>) && (!Reflectable<T>)
        inline void deserialize(T& value, J&& json){ value = json.template get<T>(); }


        // Fallback for JSON-only types: binary encoding is a size-prefixed dump of the JSON text.
        template<JsonBinaryFallback T>
        inline void serialize(const T& value, Binary::ByteArray& out){
            JSON json;
            serialize(value, json);

            std::string text = json.dump();
            serialize(text.size(), out);
            out.insert(out.end(),
                reinterpret_cast<const std::byte*>(text.data()),
                reinterpret_cast<const std::byte*>(text.data()) + text.size());
        }

        template<JsonBinaryFallback T>
        inline void deserialize(T& value, Binary::ByteView span){
            deserialize_and_advance(value, span);
        }

        template<JsonBinaryFallback T>
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){
            size_t size = 0;
            deserialize_and_advance(size, span);

            if(span.size() < size)
                throw std::runtime_error("deserialize(JsonBinaryFallback) out of bounds");

            JSON json = JSON::parse(std::string(reinterpret_cast<const char*>(span.data()), size));
            span = span.subspan(size);

            deserialize(value, json);
        }

        // Fallback for Binary-only types: JSON encoding is a base64 string of the Binary bytes.
        template<BinaryJsonFallback T, JsonLike J>
        inline void serialize(const T& value, J&& json){
            Binary::ByteArray bytes;
            serialize(value, bytes);
            json = base64_encode(bytes);
        }

        template<BinaryJsonFallback T, JsonLike J>
        inline void deserialize(T& value, J&& json){
            Binary::ByteArray bytes = base64_decode(json.template get<std::string>());
            deserialize(value, Binary::ByteView(bytes));
        }
    };
};

/// Included after in order to always be available with serializer.
#include "stl.hpp" // IWYU pragma: keep
#include "glm.hpp" // IWYU pragma: keep