#include "draft/util/asset_manager/font_loader.hpp"

#include "draft/rendering/font.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <memory>

namespace Draft {
    FontLoader::FontLoader() : BaseLoader(typeid(Font)) {}

    AssetPtr FontLoader::load_sync(Assets& assets) const {
        // Default to basic call of default filehandle constructor
        try {
            return make_asset_ptr(new Font(handle));
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(FontLoader).name(), std::to_string(e));
        }

        return make_asset_ptr<Font>(nullptr);
    }

    void FontLoader::load_async(){
        rawData = handle.read_bytes();
    }

    AssetPtr FontLoader::finish_async_gl(Assets& assets){
        return make_asset_ptr(new Font(rawData));
    }

    std::unique_ptr<BaseLoader> FontLoader::clone(const FileHandle& handle) const {
        auto ptr = std::unique_ptr<BaseLoader>(new FontLoader());
        ptr->handle = handle;
        return ptr;
    }
};