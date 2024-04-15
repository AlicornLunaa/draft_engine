#include "draft/util/json.hpp"

using namespace std;

namespace Draft {
    // Functions
    string encode_json(const JSONInterface* data){
        return data->encode();
    }

    void* decode_json(const string& data){
        return nullptr;
    }
};