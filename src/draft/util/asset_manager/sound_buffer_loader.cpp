#include "draft/util/asset_manager/sound_buffer_loader.hpp"

#include "draft/audio/sound_buffer.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <memory>

namespace Draft {
    SoundBufferLoader::SoundBufferLoader() : BaseLoader(typeid(SoundBufferLoader)) {}

    AssetPtr SoundBufferLoader::load_sync() const {
        // Default to basic call of default filehandle constructor
        try {
            return make_asset_ptr(new SoundBuffer(handle));
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(SoundBufferLoader).name(), std::to_string(e));
        }

        return make_asset_ptr<SoundBuffer>(nullptr);
    }

    void SoundBufferLoader::load_async(){
        rawData = handle.read_bytes();
    }

    AssetPtr SoundBufferLoader::finish_async_gl(){
        return make_asset_ptr(new SoundBuffer(rawData));
    }

    std::unique_ptr<BaseLoader> SoundBufferLoader::clone(const FileHandle& handle) const {
        auto ptr = std::unique_ptr<BaseLoader>(new SoundBufferLoader());
        ptr->handle = handle;
        return ptr;
    }
};