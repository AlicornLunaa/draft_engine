#include "draft/util/asset_manager/packed_texture_loader.hpp"

#include "draft/rendering/texture_packer.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace Draft {
    PackedTextureLoader::PackedTextureLoader() : BaseLoader(typeid(TexturePacker)) {}

    AssetPtr PackedTextureLoader::load_sync() const {
        // Default to basic call of default filehandle constructor
        try {
            TexturePacker* packer = new TexturePacker();
            std::vector<FileHandle> handles;

            for(std::filesystem::recursive_directory_iterator i(handle.get_path()), end; i != end; i++){
                if(!i->is_directory() && (i->path().extension() == ".png" || i->path().extension() == ".jpeg" || i->path().extension() == ".jpg")){
                    handles.push_back(i->path());
                }
            }

            packer->pack(handles);
            packer->create();
            return make_asset_ptr(packer);
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(PackedTextureLoader).name(), std::to_string(e));
        }

        return make_asset_ptr<TexturePacker>(nullptr);
    }

    void PackedTextureLoader::load_async(){
        packer = new TexturePacker();
        std::vector<FileHandle> handles;

        for(std::filesystem::recursive_directory_iterator i(handle.get_path()), end; i != end; i++){
            if(!i->is_directory() && (i->path().extension() == ".png" || i->path().extension() == ".jpeg" || i->path().extension() == ".jpg")){
                handles.push_back(i->path());
            }
        }

        packer->pack(handles);
    }

    AssetPtr PackedTextureLoader::finish_async_gl(){
        packer->create();
        return make_asset_ptr(packer);
    }

    std::unique_ptr<BaseLoader> PackedTextureLoader::clone(const FileHandle& handle) const {
        auto ptr = std::unique_ptr<BaseLoader>(new PackedTextureLoader());
        ptr->handle = handle;
        return ptr;
    }
};