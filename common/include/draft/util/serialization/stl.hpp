#pragma once
#include "draft/util/serialization/binary.hpp"
#include "draft/util/serialization/custom.hpp"
#include "draft/util/serialization/serializer.hpp"
#include <string>

template<>
struct Draft::Serializer::CustomSerializer<std::string> {
    // std::string Binary encoding: size-prefixed raw bytes, mirroring vector<K> below. Not
    // trivially copyable, so unlike arithmetic/enum types it needs this (JSON already covers it
    // via JsonTrivial, no specialization needed there).
    static void serialize(const std::string& value, Binary::ByteArray& out){
        Serializer::serialize(value.size(), out);
        out.insert(out.end(),
            reinterpret_cast<const std::byte*>(value.data()),
            reinterpret_cast<const std::byte*>(value.data()) + value.size());
    }

    static void deserialize(std::string& value, Binary::ByteView span){
        deserialize_and_advance(value, span);
    }

    static void deserialize_and_advance(std::string& value, Binary::ByteView& span){
        size_t size = 0;
        Serializer::deserialize_and_advance(size, span);

        if(span.size() < size)
            throw std::runtime_error("deserialize(std::string) out of bounds");

        value.assign(reinterpret_cast<const char*>(span.data()), size);
        span = span.subspan(size);
    }
};

template<typename K>
struct Draft::Serializer::CustomSerializer<std::vector<K>> {
    static void serialize(const std::vector<K>& array, Binary::ByteArray& out){
        // Serialize a vector array, starting with the size
        Serializer::serialize(array.size(), out);

        for(auto& val : array){
            Serializer::serialize(val, out);
        }
    }

    static void deserialize(std::vector<K>& array, Binary::ByteView span){
        deserialize_and_advance(array, span);
    }

    static void deserialize_and_advance(std::vector<K>& array, Binary::ByteView& span){
        // Deserialize a vector array, starting with the size
        size_t size = 0;
        Serializer::deserialize_and_advance(size, span);

        array.reserve(size);

        for(size_t i = 0; i < size; i++){
            K value{};
            Serializer::deserialize_and_advance(value, span);
            array.push_back(value);
        }
    }

    static void serialize(const std::vector<K>& array, Draft::JSON& json){
        json = Draft::JSON::array();

        for(auto& val : array){
            Draft::JSON child;
            Serializer::serialize(val, child);
            json.push_back(std::move(child));
        }
    }

    static void deserialize(std::vector<K>& array, const Draft::JSON& json){
        array.reserve(json.size());

        for(auto& item : json){
            Draft::JSON child = item;
            K value{};
            Serializer::deserialize(value, child);
            array.push_back(value);
        }
    }
};
