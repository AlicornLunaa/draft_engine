#pragma once

// Not meant to be included on its own - this is included from the bottom of serializer.hpp,
// which is what actually declares Binary::ByteArray/ByteView, Draft::JSON, and the
// Serializer::serialize/deserialize/deserialize_and_advance overloads this file defines
// (forward-declared there, ahead of the tiers that call into these).

namespace Draft::Serializer {
    // std::string Binary encoding: size-prefixed raw bytes, mirroring vector<K> below.
    inline void serialize(const std::string& value, Binary::ByteArray& out){
        serialize(value.size(), out);
        out.insert(out.end(),
            reinterpret_cast<const std::byte*>(value.data()),
            reinterpret_cast<const std::byte*>(value.data()) + value.size());
    }

    inline void deserialize(std::string& value, Binary::ByteView span){
        deserialize_and_advance(value, span);
    }

    inline void deserialize_and_advance(std::string& value, Binary::ByteView& span){
        size_t size = 0;
        deserialize_and_advance(size, span);

        if(span.size() < size)
            throw std::runtime_error("deserialize(std::string) out of bounds");

        value.assign(reinterpret_cast<const char*>(span.data()), size);
        span = span.subspan(size);
    }


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
        deserialize_and_advance(array, span);
    }

    template<typename K>
    inline void deserialize_and_advance(std::vector<K>& array, Binary::ByteView& span){
        // Deserialize a vector array, starting with the size
        size_t size = 0;
        deserialize_and_advance(size, span);

        array.reserve(size);

        for(size_t i = 0; i < size; i++){
            K value{};
            deserialize_and_advance(value, span);
            array.push_back(value);
        }
    }

    template<typename K>
    inline void serialize(const std::vector<K>& array, JSON& json){
        json = JSON::array();

        for(auto& val : array){
            JSON child;
            serialize(val, child);
            json.push_back(std::move(child));
        }
    }

    template<typename K>
    inline void deserialize(std::vector<K>& array, JSON& json){
        array.reserve(json.size());

        for(auto& item : json){
            JSON child = item;
            K value{};
            deserialize(value, child);
            array.push_back(value);
        }
    }
}
