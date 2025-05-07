#include "draft/util/serialization/string_serializer.hpp" // IWYU pragma: keep

namespace Draft {
    // Constants for helping with byte math
    #define HEADER_TYPE size_t
    #define HEADER_LENGTH sizeof(HEADER_TYPE)

    // String serializer specialization, this is needed because string has its own special heap data
    size_t Serializer::serialize(std::string& obj, std::vector<std::byte>& byteArray){
        // Write the first 8 bytes as a number for the length of the string
        HEADER_TYPE len = obj.length();
        std::byte* headerBytes = reinterpret_cast<std::byte*>(&len);
        
        // Append header to the byte array
        for(size_t i = 0; i < HEADER_LENGTH; i++)
            byteArray.push_back(headerBytes[i]);

        // Append the rest of the string data
        for(size_t i = 0; i < obj.length(); i++)
            byteArray.push_back(static_cast<std::byte>(obj[i]));

        // Return the new length of the field
        return HEADER_LENGTH + obj.length();
    }

    size_t Serializer::deserialize(std::string& obj, std::byte* bytePtr){
        // Read the first 8 bytes as a number for the length of the string
        HEADER_TYPE len = *reinterpret_cast<const HEADER_TYPE*>(bytePtr);

        // Create the string from the buffer
        obj = std::string(reinterpret_cast<const char*>(bytePtr + HEADER_LENGTH), len);

        // Return new length
        return HEADER_LENGTH + obj.length();
    }

    void Serializer::serialize(std::string& obj, JSON& json){
        // JSON is fine already because it doesn't deal with memory.
        json = obj;
    }

    void Serializer::deserialize(std::string& obj, JSON& json){
        // JSON is fine already because it doesn't deal with memory.
        obj = json.template get<std::string>();
    }
};