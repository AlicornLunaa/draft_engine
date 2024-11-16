#include "draft/util/asset_manager/json_loader.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep

using json = nlohmann::json;

namespace Draft {
    JSONLoader::JSONLoader() : BaseLoader(typeid(json)) {}

    AssetPtr JSONLoader::load_sync() const {
        // Default to basic call of default filehandle constructor
        try {
            json* jsonPtr = new json();
            *jsonPtr = json::parse(handle.read_string());
            return make_asset_ptr(jsonPtr);
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(JSONLoader).name(), std::to_string(e));
        }

        return make_asset_ptr<json>(nullptr);
    }

    void JSONLoader::load_async(){
        data = new json();
        *data = json::parse(handle.read_string());
    }

    AssetPtr JSONLoader::finish_async_gl(){
        return make_asset_ptr(data);
    }

    std::unique_ptr<BaseLoader> JSONLoader::clone(const FileHandle& handle) const {
        auto ptr = std::unique_ptr<JSONLoader>(new JSONLoader());
        ptr->handle = handle;
        return ptr;
    }
};