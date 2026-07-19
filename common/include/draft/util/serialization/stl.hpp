#pragma once
#include "draft/util/serialization/binary.hpp"
#include "draft/util/serialization/custom.hpp"
#include "draft/util/serialization/serializer.hpp"
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

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

template<typename... Ts>
struct Draft::Serializer::CustomSerializer<std::variant<Ts...>> {
    using Variant = std::variant<Ts...>;

    // Binary encoding: the active index (a size_t, mirroring how the vector<K> specialization
    // above encodes its own size), followed by the active alternative's own serialized value.
    static void serialize(const Variant& variant, Binary::ByteArray& out){
        Serializer::serialize(variant.index(), out);
        std::visit([&out](const auto& value){ Serializer::serialize(value, out); }, variant);
    }

    static void deserialize(Variant& variant, Binary::ByteView span){
        deserialize_and_advance(variant, span);
    }

    static void deserialize_and_advance(Variant& variant, Binary::ByteView& span){
        size_t index = 0;
        Serializer::deserialize_and_advance(index, span);
        emplace_alternative(variant, index, std::index_sequence_for<Ts...>{});

        std::visit([&span](auto& value){ Serializer::deserialize_and_advance(value, span); }, variant);
    }

    static void serialize(const Variant& variant, Draft::JSON& json){
        Serializer::serialize(variant.index(), json["index"]);
        std::visit([&json](const auto& value){ Serializer::serialize(value, json["value"]); }, variant);
    }

    static void deserialize(Variant& variant, const Draft::JSON& json){
        size_t index = json.at("index").template get<size_t>();
        emplace_alternative(variant, index, std::index_sequence_for<Ts...>{});

        std::visit([&json](auto& value){ Serializer::deserialize(value, json.at("value")); }, variant);
    }

private:
    // Default-constructs whichever alternative `index` names (found by matching it against each
    // compile-time Is in turn), since the runtime index has to select from a compile-time list of
    // alternative types.
    template<size_t... Is>
    static void emplace_alternative(Variant& variant, size_t index, std::index_sequence<Is...>){
        if(index >= sizeof...(Ts))
            throw std::runtime_error("deserialize(std::variant) invalid index");

        ((index == Is ? (void)variant.template emplace<Is>() : void()), ...);
    }
};

template<typename T>
struct Draft::Serializer::CustomSerializer<std::optional<T>> {
    // Binary encoding: a has-value flag, then the value itself if present, mirroring
    // CameraComponent's old hand-written "hasCamera" flag.
    static void serialize(const std::optional<T>& value, Binary::ByteArray& out){
        bool hasValue = value.has_value();
        Serializer::serialize(hasValue, out);

        if(hasValue)
            Serializer::serialize(*value, out);
    }

    static void deserialize(std::optional<T>& value, Binary::ByteView span){
        deserialize_and_advance(value, span);
    }

    static void deserialize_and_advance(std::optional<T>& value, Binary::ByteView& span){
        bool hasValue = false;
        Serializer::deserialize_and_advance(hasValue, span);

        if(!hasValue){
            value.reset();
            return;
        }

        T inner{};
        Serializer::deserialize_and_advance(inner, span);
        value = std::move(inner);
    }

    // JSON encoding: null when empty, the value itself otherwise.
    static void serialize(const std::optional<T>& value, Draft::JSON& json){
        if(value)
            Serializer::serialize(*value, json);
        else
            json = nullptr;
    }

    static void deserialize(std::optional<T>& value, const Draft::JSON& json){
        if(json.is_null()){
            value.reset();
            return;
        }

        T inner{};
        Serializer::deserialize(inner, json);
        value = std::move(inner);
    }
};
