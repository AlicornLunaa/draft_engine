#pragma once

#include <bit>
#include "draft/util/json.hpp"

// Main declarations here
namespace Draft {
    namespace Binary {
        // Build variables
        inline constexpr bool IS_LITTLE_ENDIAN = std::endian::native == std::endian::little;
    
        // Helpers
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
    
        template<typename T>
        inline T from_little_endian(T value){
            return to_little_endian(value);
        }

        // Binary operations
        template<typename T>
        inline void write(std::vector<std::byte>& buffer, const T& value){
            // Simply writes the buffer with the value
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
            T littleEndian = to_little_endian(value);
            const auto* bytes = reinterpret_cast<const std::byte*>(&littleEndian);
            buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
        }

        template<typename T>
        inline void read(std::span<const std::byte>& span, T& value){
            // Simply assigns the value with the data from the span.
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

            if(span.size() < sizeof(T))
                throw std::runtime_error("read() out of bounds");
            
            std::memcpy(&value, span.data(), sizeof(T));
            value = from_little_endian(value);
            span = span.subspan(sizeof(T)); // Advance the span as convention says
        }
    };

    namespace Serializer {
        // Serializable types
        template<typename T>
        concept Serializable = requires(T object, std::vector<std::byte>& buffer, std::span<const std::byte>& span, Draft::JSON& json){
            { T::serialize(object, buffer) } -> std::convertible_to<void>;
            { T::deserialize(object, span) } -> std::convertible_to<void>;
            { T::serialize(object, json) } -> std::convertible_to<void>;
            { T::deserialize(object, json) } -> std::convertible_to<void>;
        };

        // Default for trivial types
        template<typename T> requires std::is_trivially_copyable_v<T>
        inline void serialize(const T& value, std::vector<std::byte>& out){ write(out, value); }

        template<typename T> requires std::is_trivially_copyable_v<T>
        inline void deserialize(T& value, std::span<const std::byte>& span){ read(span, value); }

        // Default for JSON-serializable trivials
        template<typename T> requires std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string>
        inline void serialize(const T& value, JSON& json){ json = value; }

        template<typename T> requires std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string>
        inline void deserialize(T& value, JSON& json){ value = json.template get<T>(); }
    };
};