#pragma once

#include "draft/util/serialization/binary.hpp"
#include <cstdint>
#include <string>

namespace Draft {
    namespace Serializer {
        /**
        * @brief Base64 helpers backing the BinaryJsonFallback tier below. Not part of the public
        * Binary/JSON API, text JSON has no native byte-string type, so this is the encoding used
        * to smuggle a Binary payload through it.
        */
        inline std::string base64_encode(Binary::ByteView bytes){
            static constexpr char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string out;
            out.reserve(((bytes.size() + 2) / 3) * 4);

            size_t i = 0;
            for(; i + 3 <= bytes.size(); i += 3){
                uint32_t chunk = (std::to_integer<uint32_t>(bytes[i]) << 16)
                    | (std::to_integer<uint32_t>(bytes[i + 1]) << 8)
                    | std::to_integer<uint32_t>(bytes[i + 2]);

                out.push_back(table[(chunk >> 18) & 0x3F]);
                out.push_back(table[(chunk >> 12) & 0x3F]);
                out.push_back(table[(chunk >> 6) & 0x3F]);
                out.push_back(table[chunk & 0x3F]);
            }

            size_t remaining = bytes.size() - i;
            if(remaining == 1){
                uint32_t chunk = std::to_integer<uint32_t>(bytes[i]) << 16;
                out.push_back(table[(chunk >> 18) & 0x3F]);
                out.push_back(table[(chunk >> 12) & 0x3F]);
                out.append("==");
            } else if(remaining == 2){
                uint32_t chunk = (std::to_integer<uint32_t>(bytes[i]) << 16) | (std::to_integer<uint32_t>(bytes[i + 1]) << 8);
                out.push_back(table[(chunk >> 18) & 0x3F]);
                out.push_back(table[(chunk >> 12) & 0x3F]);
                out.push_back(table[(chunk >> 6) & 0x3F]);
                out.push_back('=');
            }

            return out;
        }

        inline Binary::ByteArray base64_decode(const std::string& text){
            auto value = [](char c) -> int {
                if(c >= 'A' && c <= 'Z') return c - 'A';
                if(c >= 'a' && c <= 'z') return c - 'a' + 26;
                if(c >= '0' && c <= '9') return c - '0' + 52;
                if(c == '+') return 62;
                if(c == '/') return 63;
                return -1;
            };

            Binary::ByteArray out;
            out.reserve((text.size() / 4) * 3);

            uint32_t chunk = 0;
            int bits = 0;
            for(char c : text){
                int v = value(c);
                if(v < 0) continue; // padding ('=') or stray whitespace, neither carries bits

                chunk = (chunk << 6) | static_cast<uint32_t>(v);
                bits += 6;

                if(bits >= 8){
                    bits -= 8;
                    out.push_back(static_cast<std::byte>((chunk >> bits) & 0xFF));
                }
            }

            return out;
        }
    }
}