#pragma once

#include <bit>
#include <cstring>
#include <span>
#include <stdexcept>
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
    }
}