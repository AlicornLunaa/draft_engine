#include "draft/util/asset_manager/image_loader.hpp"

#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <memory>

namespace Draft {
    ImageLoader::ImageLoader() : BaseLoader(typeid(Image)) {}

    AssetPtr ImageLoader::load_sync() const {
        // Default to basic call of default filehandle constructor
        try {
            return make_asset_ptr(new Image(handle));
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(ImageLoader).name(), std::to_string(e));
        }

        return make_asset_ptr<Image>(nullptr);
    }

    void ImageLoader::load_async(){
        imgPtr = new Image(handle);
    }

    AssetPtr ImageLoader::finish_async_gl(){
        return make_asset_ptr(imgPtr);
    }

    std::unique_ptr<BaseLoader> ImageLoader::clone(const FileHandle& handle) const {
        auto ptr = std::unique_ptr<BaseLoader>(new ImageLoader());
        ptr->handle = handle;
        return ptr;
    }
};