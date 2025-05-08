#include "draft/util/asset_manager/collider_loader.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    ColliderLoader::ColliderLoader() : BaseLoader(typeid(Collider)) {}

    AssetPtr ColliderLoader::load_sync(Assets& assets) const {
        // Default to basic call of default filehandle constructor
        try {
            Collider* collider = new Collider(JSON::parse(handle.read_string()));
            return make_asset_ptr(collider);
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(ColliderLoader).name(), std::to_string(e));
        }

        return make_asset_ptr<JSON>(nullptr);
    }

    void ColliderLoader::load_async(){
        collider = new Collider(JSON::parse(handle.read_string()));
    }

    AssetPtr ColliderLoader::finish_async_gl(Assets& assets){
        return make_asset_ptr(collider);
    }

    std::unique_ptr<BaseLoader> ColliderLoader::clone(const FileHandle& handle) const {
        auto ptr = std::unique_ptr<ColliderLoader>(new ColliderLoader());
        ptr->handle = handle;
        return ptr;
    }
};