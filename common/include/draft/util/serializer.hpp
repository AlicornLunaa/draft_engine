#pragma once

#include "draft/util/json.hpp"

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

        // Default for complex types
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

        // Default for JSON-serializable complex types
        template<JsonSerializable T>
        inline void serialize(const T& value, JSON& json){ T::serialize(value, json); }

        template<JsonSerializable T>
        inline void deserialize(T& value, JSON& json){ T::deserialize(value, json); }


        // Default for trivial types
        template<typename T> requires std::is_trivially_copyable_v<T>
        inline void serialize(const T& value, Binary::ByteArray& out){ Binary::write(out, value); }

        template<typename T> requires std::is_trivially_copyable_v<T>
        inline void deserialize(T& value, Binary::ByteView span){ Binary::read(span, value); }

        template<typename T> requires std::is_trivially_copyable_v<T>
        inline void deserialize_and_advance(T& value, Binary::ByteView& span){
            deserialize(value, span);
            span = span.subspan(sizeof(T));
        }

        // Default for JSON-serializable trivials
        template<typename T> requires std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string>
        inline void serialize(const T& value, JSON& json){ json = value; }

        template<typename T> requires std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string>
        inline void deserialize(T& value, JSON& json){ value = json.template get<T>(); }


        // Basic included non-trivial serializers
        template<typename K>
        inline void serialize(const std::vector<K>& array, Binary::ByteArray& out){
            // Serialize a vector array, starting with the size
            serialize(array.size(), out);

            for(auto& val : array){
                serialize(val, out);
            }
        }

        template<typename K>
        inline void deserialize(std::vector<K>& array, Binary::ByteView span){
            // Serialize a vector array, starting with the size
            size_t size = 0;
            deserialize_and_advance(size, span);

            array.reserve(size);

            for(size_t i = 0; i < size; i++){
                K value{};
                deserialize_and_advance(value, span);
                array.push_back(value);
            }
        }
    };
};
