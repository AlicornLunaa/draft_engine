#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/particle_system.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/texture_packer.hpp"
#include "draft/util/asset_manager/font_loader.hpp"
#include "draft/util/asset_manager/generic_loader.hpp"
#include "draft/util/asset_manager/image_loader.hpp"
#include "draft/util/asset_manager/packed_texture_loader.hpp"
#include "draft/util/asset_manager/particle_loader.hpp"
#include "draft/util/asset_manager/json_loader.hpp"
#include "draft/util/asset_manager/sound_buffer_loader.hpp"
#include "draft/util/asset_manager/texture_loader.hpp"
#include "draft/util/json.hpp"
#include "draft/util/logger.hpp"
#include "tracy/Tracy.hpp"

namespace Draft {
    // Private functions
    void Assets::load_async_queue(std::queue<std::unique_ptr<BaseLoader>>& loadQueue, std::queue<std::unique_ptr<BaseLoader>>& finishQueue, size_t totalAssets, float* progress, std::mutex& mut){
        // Variables
        size_t currentLoaded = 0;

        // Error handle
        if(loadQueue.empty()){
            // No queue, end this thread as a success
            std::lock_guard<std::mutex> guard(mut);
            *progress = 1.f;
            return;
        }

        while(!loadQueue.empty()){
            if(loadQueue.front() == nullptr){
                loadQueue.pop();
                continue;
            }

            auto& loader = loadQueue.front();
            loader->load_async();

            {
                // Prevent race conditions
                std::lock_guard<std::mutex> guard(mut);

                // Try queuing up the resources that need finalized
                finishQueue.push(std::move(loader));

                // Update percentage
                currentLoaded++;
                *progress = currentLoaded / (float)totalAssets;
            }

            loadQueue.pop();
        }
    }

    void Assets::finish_async_queue(){
        // Prevent race conditions
        std::lock_guard<std::mutex> guard(asyncMutex);

        while(!stage2Queue.empty()){
            auto& loader = stage2Queue.front();
            resources.insert_or_assign(ResourceKey{loader->handle.get_path(), loader->type}, loader->finish_async_gl(*this));
            stage2Queue.pop();
        }
    }

    void Assets::register_placeholders(){
        register_placeholder(new Texture(Image({1, 1}, {1, 0, 1, 1}, RGB)));
        register_placeholder(new Shader("assets/shaders/missing_shader"));
    }

    // Constructors
    Assets::Assets(){
        // Setup loaders
        register_loader<Image>(new ImageLoader());
        register_loader<Font>(new FontLoader());
        register_loader<SoundBuffer>(new SoundBufferLoader());
        register_loader<Model>(new GenericSyncLoader<Model>());
        register_loader<Shader>(new GenericSyncLoader<Shader>());
        register_loader<Texture>(new TextureLoader());
        register_loader<ParticleProps>(new ParticleLoader());
        register_loader<JSON>(new JSONLoader());
        register_loader<TexturePacker>(new PackedTextureLoader());
    }

    // Functions
    void Assets::load(){
        // Profiler
        ZoneScopedN("asset_load");

        // Load everything in the load queue
        register_placeholders();
        
        while(!stage1Queue.empty()){
            auto& loader = stage1Queue.front();
            resources.insert_or_assign(ResourceKey{loader->handle.get_path(), loader->type}, loader->load_sync(*this));
            stage1Queue.pop();
        }
    }

    void Assets::load_async(){
        // Placeholders
        register_placeholders();

        // Get total assets to keep a percentage
        size_t totalAssets = 0;
        loadingProgress = 0.f;
        loadingAsyncronously = true;
        totalAssets += stage1Queue.size();

        // Spawn thread to run the asyncronous load
        std::thread loadThread([this, totalAssets](){ load_async_queue(stage1Queue, stage2Queue, totalAssets, &loadingProgress, asyncMutex); });
        loadThread.detach();
    }

    bool Assets::poll_async(){
        bool loadingFinished = get_loading_progress() >= 1.f;

        if(loadingFinished && loadingAsyncronously){
            finish_async_queue();
            loadingAsyncronously = false;
        }

        return loadingFinished;
    }

    float Assets::get_loading_progress(){
        std::lock_guard<std::mutex> guard(asyncMutex);
        return loadingProgress;
    }

    void Assets::reload(){
        // Profiler
        ZoneScopedN("asset_reload");

        Logger::print(Level::INFO, "Asset Manager", "Reloading...");

        for(auto& [key, assetPtr] : resources){
            auto const& loader = loaders[key.second];
            if(!loader) continue;
            stage1Queue.push(loader->clone(FileHandle::automatic(key.first)));
        }
        load();

        Logger::print_raw("Complete\n");
    }
}