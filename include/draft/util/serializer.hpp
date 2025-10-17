#pragma once

#include <bit>
#include <cstddef>
#include <vector>
#include "draft/util/json.hpp"

// Main declarations here
namespace Draft {
    namespace Binary {
        // Build variables and types
        inline constexpr bool IS_LITTLE_ENDIAN = std::endian::native == std::endian::little;
        typedef std::vector<std::byte> ByteArray;
        typedef std::span<std::byte> ByteSpan;
        typedef std::span<const std::byte> ConstByteSpan;
    
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
        inline void write(ByteArray& buffer, const T& value){
            // Simply writes the buffer with the value
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
            T littleEndian = to_little_endian(value);
            const auto* bytes = reinterpret_cast<const std::byte*>(&littleEndian);
            buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
        }

        template<typename T>
        inline void read(ConstByteSpan span, T& value){
            // Simply assigns the value with the data from the span.
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

            if(span.size() < sizeof(T))
                throw std::runtime_error("read() out of bounds");
            
            std::memcpy(&value, span.data(), sizeof(T));
            value = from_little_endian(value);
        }
        
        template<typename T>
        inline void read_and_advance(ConstByteSpan& span, T& value){
            // Simply assigns the value with the data from the span.
            read(span, value);

            // Advance the span for ease of use if its modifiable
            span = span.subspan(sizeof(T));
        }
    };

    namespace Serializer {
        // Serializable types
        template<typename T>
        concept Serializable = requires(T object, Binary::ByteArray& buffer, Binary::ConstByteSpan& span, Draft::JSON& json){
            { T::serialize(object, buffer) } -> std::convertible_to<void>;
            { T::deserialize(object, span) } -> std::convertible_to<void>;
            { T::serialize(object, json) } -> std::convertible_to<void>;
            { T::deserialize(object, json) } -> std::convertible_to<void>;
        };

        // Default for complex types
        template<Serializable T>
        inline void serialize(const T& value, Binary::ByteArray& out){ T::serialize(value, out); }

        template<Serializable T>
        inline void deserialize(T& value, Binary::ConstByteSpan span){ T::deserialize(span, value); }

        template<Serializable T>
        inline void deserialize_and_advance(T& value, Binary::ConstByteSpan& span){
            T::deserialize(value, span);
            span = span.subspan(sizeof(T));
        }

        // Default for JSON-serializable complex types
        template<Serializable T>
        inline void serialize(const T& value, JSON& json){ T::serialize(value, json); }

        template<Serializable T>
        inline void deserialize(T& value, JSON& json){ T::deserialize(value, json); }


        // Default for trivial types
        template<typename T> requires std::is_trivially_copyable_v<T>
        inline void serialize(const T& value, Binary::ByteArray& out){ Binary::write(out, value); }

        template<typename T> requires std::is_trivially_copyable_v<T>
        inline void deserialize(T& value, Binary::ConstByteSpan span){ Binary::read(span, value); }

        template<typename T> requires std::is_trivially_copyable_v<T>
        inline void deserialize_and_advance(T& value, Binary::ConstByteSpan& span){
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
        inline void deserialize(std::vector<K>& array, Binary::ConstByteSpan span){
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