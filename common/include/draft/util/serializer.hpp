#pragma once

#include "draft/util/json.hpp"
#include "draft/util/reflectable.hpp"

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Draft {
    /**
     * @brief Low-level little-endian binary encoding helpers.
     *
     * These operate on raw byte buffers/spans and only know about fixed-size, trivially
     * copyable data. Serializer builds the higher-level, type-driven API on top of this.
     */
    namespace Binary {
        /**
         * @brief True if this platform's native byte order is already little-endian.
         */
        inline constexpr bool IS_LITTLE_ENDIAN = std::endian::native == std::endian::little;

        using ByteArray = std::vector<std::byte>;
        using ByteSpan = std::span<std::byte>;
        using ByteView = std::span<const std::byte>;

        /**
         * @brief Converts @p value to little-endian byte order.
         * Non-integral types are returned unchanged (endianness only applies to integers here).
         */
        template<typename T>
        inline T to_little_endian(T value){
            // This guarantees little endianness no matter the platform
            if constexpr (std::is_integral_v<T> && !IS_LITTLE_ENDIAN){
                T result{};

                auto src = reinterpret_cast<const unsigned char*>(&value);
                auto dest = reinterpret_cast<unsigned char*>(&result);

                for(size_t i = 0; i < sizeof(T); i++){
                    // Swap bytes
                    dest[i] = src[sizeof(T) - 1 - i];
                }

                return result;
            }

            // Otherwise, if its not an integral type then just return the value given
            return value;
        }

        /**
         * @brief Converts @p value from little-endian byte order back to the platform's native order.
         * Byte-swapping is its own inverse, so this is identical to to_little_endian().
         */
        template<typename T>
        inline T from_little_endian(T value){
            return to_little_endian(value);
        }

        /**
         * @brief Appends @p value to @p buffer in little-endian byte order.
         */
        template<typename T>
        inline void write(ByteArray& buffer, const T& value){
            // Simply writes the buffer with the value
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
            T littleEndian = to_little_endian(value);
            const auto* bytes = reinterpret_cast<const std::byte*>(&littleEndian);
            buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
        }

        /**
         * @brief Reads a @p T out of the front of @p span into @p value, without advancing it.
         * @throws std::runtime_error if @p span has fewer than sizeof(T) bytes.
         */
        template<typename T>
        inline void read(ByteView span, T& value){
            // Simply assigns the value with the data from the span.
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

            if(span.size() < sizeof(T))
                throw std::runtime_error("read() out of bounds");

            std::memcpy(&value, span.data(), sizeof(T));
            value = from_little_endian(value);
        }

        /**
         * @brief Same as read(), but advances @p span past the bytes just consumed.
         */
        template<typename T>
        inline void read_and_advance(ByteView& span, T& value){
            // Simply assigns the value with the data from the span.
            read(span, value);

            // Advance the span for ease of use if its modifiable
            span = span.subspan(sizeof(T));
        }
    };

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
         */
        template<typename T>
        concept JsonSerializable = requires(T object, Draft::JSON& json){
            { T::serialize(object, json) } -> std::convertible_to<void>;
            { T::deserialize(object, json) } -> std::convertible_to<void>;
        };

        /**
         * @brief Satisfied by types that support both Binary and JSON (de)serialization.
         */
        template<typename T>
        concept Serializable = requires(T object, Binary::ByteArray& buffer, Binary::ByteView& span, Draft::JSON& json){
            { T::serialize(object, buffer) } -> std::convertible_to<void>;
            { T::deserialize(object, span) } -> std::convertible_to<void>;
            { T::serialize(object, json) } -> std::convertible_to<void>;
            { T::deserialize(object, json) } -> std::convertible_to<void>;
        };

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
        struct CustomSerializer; // TODO: Hoist out so the containers can specialize without including this file, allowing this file to include the containers so the containers are always available by default

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
        concept CustomJsonSerializable = requires(T object, Draft::JSON& json){
            { CustomSerializer<T>::serialize(object, json) } -> std::convertible_to<void>;
            { CustomSerializer<T>::deserialize(object, json) } -> std::convertible_to<void>;
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
        template<BinarySerializable T> void deserialize_and_advance(T& value, Binary::ByteView& span);

        template<JsonSerializable T> void serialize(const T& value, JSON& json);
        template<JsonSerializable T> void deserialize(T& value, JSON& json);

        // Explicit tier: out-of-line via a CustomSerializer<T> specialization
        template<typename T> requires CustomBinarySerializable<T> && (!BinarySerializable<T>)
        void serialize(const T& value, Binary::ByteArray& out);
        template<typename T> requires CustomBinarySerializable<T> && (!BinarySerializable<T>)
        void deserialize(T& value, Binary::ByteView span);
        template<typename T> requires CustomBinarySerializableWithAdvance<T> && (!BinarySerializable<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);
        template<typename T> requires CustomBinarySerializable<T> && (!CustomBinarySerializableWithAdvance<T>) && (!BinarySerializable<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);

        template<typename T> requires CustomJsonSerializable<T> && (!JsonSerializable<T>)
        void serialize(const T& value, JSON& json);
        template<typename T> requires CustomJsonSerializable<T> && (!JsonSerializable<T>)
        void deserialize(T& value, JSON& json);

        // Reflectable tier
        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        void serialize(const T& value, Binary::ByteArray& out);
        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        void deserialize(T& value, Binary::ByteView span);
        template<typename T> requires Reflectable<T> && (!BinarySerializable<T>) && (!CustomBinarySerializable<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);

        template<typename T> requires Reflectable<T> && (!JsonSerializable<T>) && (!CustomJsonSerializable<T>)
        void serialize(const T& value, JSON& json);
        template<typename T> requires Reflectable<T> && (!JsonSerializable<T>) && (!CustomJsonSerializable<T>)
        void deserialize(T& value, JSON& json);

        // Trivial tier
        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        void serialize(const T& value, Binary::ByteArray& out);
        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        void deserialize(T& value, Binary::ByteView span);
        template<TriviallySerializable T> requires (!BinarySerializable<T>) && (!CustomBinarySerializable<T>) && (!Reflectable<T>)
        void deserialize_and_advance(T& value, Binary::ByteView& span);

        template<JsonTrivial T> requires (!JsonSerializable<T>) && (!CustomJsonSerializable<T>) && (!Reflectable<T>)
        void serialize(const T& value, JSON& json);
        template<JsonTrivial T> requires (!JsonSerializable<T>) && (!CustomJsonSerializable<T>) && (!Reflectable<T>)
        void deserialize(T& value, JSON& json);
        ///@}

        // Default for complex types with explicit binary functions
        template<BinarySerializable T>
        inline void serialize(const T& value, Binary::ByteArray& out){ T::serialize(value, out); }

        template<BinarySerializable T>
        inline void deserialize(T& value, Binary::ByteView span){ T::deserialize(value, span); }

        /**
         * @brief Deserializes a BinarySerializable @p value and advances @p span past it.
         * Assumes T::deserialize consumes exactly sizeof(T) bytes; a type with a variable-length
         * binary encoding needs to advance its own span manually instead of using this helper.
         */
        template<BinarySerializable T>
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){
            T::deserialize(value, span);
            span = span.subspan(sizeof(T));
        }

        // Default for explicit JSON-serializable complex types
        template<JsonSerializable T>
        inline void serialize(const T& value, JSON& json){ T::serialize(value, json); }

        template<JsonSerializable T>
        inline void deserialize(T& value, JSON& json){ T::deserialize(value, json); }


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

        template<typename T> requires CustomJsonSerializable<T> && (!JsonSerializable<T>)
        inline void serialize(const T& value, JSON& json){ CustomSerializer<T>::serialize(value, json); }

        template<typename T> requires CustomJsonSerializable<T> && (!JsonSerializable<T>)
        inline void deserialize(T& value, JSON& json){ CustomSerializer<T>::deserialize(value, json); }


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
        template<typename T> requires Reflectable<T> && (!JsonSerializable<T>) && (!CustomJsonSerializable<T>)
        inline void serialize(const T& value, JSON& json){
            for_each_field(value, [&](std::string_view name, const auto& field){
                JSON child;
                serialize(field, child);
                json[std::string(name)] = static_cast<nlohmann::json&&>(child);
            });
        }

        template<typename T> requires Reflectable<T> && (!JsonSerializable<T>) && (!CustomJsonSerializable<T>)
        inline void deserialize(T& value, JSON& json){
            for_each_field(value, [&](std::string_view name, auto& field){
                // json.at() returns a base nlohmann::json&, which can't bind to the JSON&
                // (Draft::JSON&) parameter below, copy into a real JSON first.
                JSON child = json.at(std::string(name));
                deserialize(field, child);
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
        template<JsonTrivial T> requires (!JsonSerializable<T>) && (!CustomJsonSerializable<T>) && (!Reflectable<T>)
        inline void serialize(const T& value, JSON& json){ json = value; }

        template<JsonTrivial T> requires (!JsonSerializable<T>) && (!CustomJsonSerializable<T>) && (!Reflectable<T>)
        inline void deserialize(T& value, JSON& json){ value = json.template get<T>(); }
    };
};