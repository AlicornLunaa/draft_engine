#include "draft/util/json.hpp"

namespace Draft {
    std::string encode_json(const JSONInterface& data) {
        return data.encode();
    }

    void decode_json(JSONInterface& data, const std::string& str) {
        data.decode(str);
    }
};
