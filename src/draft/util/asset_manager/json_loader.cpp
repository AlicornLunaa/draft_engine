#include "draft/util/asset_manager/json_loader.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    JSONLoader::JSONLoader() : BaseLoader(typeid(JSON)) {}

    AssetPtr JSONLoader::load_sync(Assets& assets) const {
        // Default to basic call of default filehandle constructor
        try {
            JSON* jsonPtr = new JSON();
            *jsonPtr = JSON::parse(handle.read_string());
            return make_asset_ptr(jsonPtr);
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(JSONLoader).name(), std::to_string(e));
        }

        return make_asset_ptr<JSON>(nullptr);
    }

    void JSONLoader::load_async(){
        data = new JSON();
        *data = JSON::parse(handle.read_string());
    }

    AssetPtr JSONLoader::finish_async_gl(Assets& assets){
        return make_asset_ptr(data);
    }

    std::unique_ptr<BaseLoader> JSONLoader::clone(const FileHandle& handle) const {
        auto ptr = std::unique_ptr<JSONLoader>(new JSONLoader());
        ptr->handle = handle;
        return ptr;
    }
};