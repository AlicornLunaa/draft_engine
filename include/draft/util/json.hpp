#pragma once

#include <string>

namespace Draft {
    // Interface
    class JSONInterface {
    public:
        // Constructors
        JSONInterface();
        virtual ~JSONInterface();

        // Functions
        virtual std::string encode() const = 0;
        virtual void* decode(const std::string& data) = 0;
    };

    // Functions
    std::string encode_json(const JSONInterface& data);
    void* decode_json(const std::string& data);
};