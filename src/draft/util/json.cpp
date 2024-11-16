#include "draft/util/json.hpp"
#include <functional>
#include <typeindex>
#include <unordered_map>

using namespace std;

namespace Draft {
    // Variables
    std::unordered_map<std::type_index, std::function<void*(const string& str)>> decodeFunctionMap;

    // Class

    // Functions
    string encode_json(const JSONInterface* data){
        return data->encode();
    }

    void* decode_json(const string& data){
        return nullptr;
    }
};