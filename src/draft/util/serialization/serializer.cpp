#include "draft/util/serialization/serializer.hpp"
#include "draft/util/serialization/serializable.hpp"
#include <cstddef>
#include <cstring>

namespace Draft {
    // Serializable type
    template<>
    void Serializer::serialize(Serializable& obj, std::vector<std::byte>& byteArray){
        // Save each type
        for(auto& [key, data] : obj.fieldMap){
            auto& [ptr, size] = data;
            char* bytes = new char[size];
            std::memcpy(bytes, ptr, size);

            for(size_t i = 0; i < size; i++)
                byteArray.push_back(static_cast<std::byte>(bytes[i]));

            delete[] bytes;
        }
    }

    template<>
    void Serializer::deserialize(Serializable& obj, std::vector<std::byte>& byteArray){
        // Load each type
        size_t offset = 0;

        for(auto& [key, data] : obj.fieldMap){
            auto& [ptr, size] = data;
            std::memcpy(ptr, byteArray.data() + offset, size);
            offset += size;
        }
    }
    
    template<>
    void Serializer::serialize(Serializable& obj, nlohmann::json& json){
        // Save each type
        for(auto& [key, data] : obj.fieldMap){
            auto& [ptr, size] = data;
            char* bytes = new char[size];
            std::memcpy(bytes, ptr, size);
            json[key] = std::string(bytes, size);
            delete[] bytes;
        }
    }

    template<>
    void Serializer::deserialize(Serializable& obj, nlohmann::json& json){
        // Load each type
        for(auto& [key, data] : obj.fieldMap){
            std::string strData = json[key];
            const char* arr = strData.data();

            auto& [ptr, size] = data;
            std::memcpy(ptr, arr, size);
        }
    }
};