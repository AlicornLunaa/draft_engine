#include "draft/util/asset_manager/texture_loader.hpp"

#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <memory>

namespace Draft {
    TextureLoader::TextureLoader() : BaseLoader(typeid(Texture)) {}

    AssetPtr TextureLoader::load_sync() const {
        // Default to basic call of default filehandle constructor
        try {
            return make_asset_ptr(new Texture(handle));
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(TextureLoader).name(), std::to_string(e));
        }

        return make_asset_ptr<Texture>(nullptr);
    }

    void TextureLoader::load_async(){
        img = std::make_unique<Image>(handle, true);
    }

    AssetPtr TextureLoader::finish_async_gl(){
        Texture* tex = new Texture(*img);
        tex->set_reloadable(handle);
        return make_asset_ptr(tex);
    }

    std::unique_ptr<BaseLoader> TextureLoader::clone(const FileHandle& handle) const {
        auto ptr = std::unique_ptr<BaseLoader>(new TextureLoader());
        ptr->handle = handle;
        return ptr;
    }
};