#include "draft/util/asset_manager/json_loader.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep

using json = nlohmann::json;

namespace Draft {
    std::shared_ptr<void> JSONLoader::load_sync() const {
        // Default to basic call of default filehandle constructor
        try {
            auto ptr = std::shared_ptr<json>(new json(), [](void* ptr){ delete static_cast<json*>(ptr); });
            *ptr = json::parse(handle.read_string());
            return ptr;
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(JSONLoader).name(), std::to_string(e));
        }

        return nullptr;
    }

    void JSONLoader::load_async(){
        data = new json();
        *data = json::parse(handle.read_string());
    }

    std::shared_ptr<void> JSONLoader::finish_async_gl(){
        auto ptr = std::shared_ptr<json>(data, [](void* ptr){ delete static_cast<json*>(ptr); });
        return ptr;
    }

    BaseLoader* JSONLoader::clone(const FileHandle& handle) const {
        auto* ptr = new JSONLoader();
        ptr->handle = handle;
        return ptr;
    }
};