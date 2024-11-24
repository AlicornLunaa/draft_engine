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
#include "draft/util/asset_manager/generic_loader.hpp"
#include "draft/util/asset_manager/particle_loader.hpp"
#include "draft/util/asset_manager/json_loader.hpp"
#include "draft/util/logger.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep

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
            resources.insert_or_assign(ResourceKey{loader->handle.get_path(), loader->type}, loader->finish_async_gl());
            stage2Queue.pop();
        }
    }

    void Assets::register_placeholders(){
        register_placeholder(new Texture(Image(1, 1, {1, 0, 1, 1}, ColorSpace::RGB)));
        register_placeholder(new Shader("assets/shaders/missing_shader"));
    }

    // Constructors
    Assets::Assets(){
        // Setup loaders
        register_loader<Image>(new GenericLoader<Image>());
        register_loader<Font>(new GenericLoader<Font>());
        register_loader<SoundBuffer>(new GenericSyncLoader<SoundBuffer>());
        register_loader<Model>(new GenericSyncLoader<Model>());
        register_loader<Shader>(new GenericSyncLoader<Shader>());
        register_loader<Texture>(new GenericSyncLoader<Texture>());
        register_loader<ParticleProps>(new ParticleLoader());
        register_loader<nlohmann::json>(new JSONLoader());
    }

    // Public vars
    Assets Assets::manager;

    // Functions
    void Assets::load(){
        // Load everything in the load queue
        register_placeholders();
        
        while(!stage1Queue.empty()){
            auto& loader = stage1Queue.front();
            resources.insert_or_assign(ResourceKey{loader->handle.get_path(), loader->type}, loader->load_sync());
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
        Logger::print(Level::INFO, "Asset Manager", "Reloading...");

        for(auto& [key, assetPtr] : resources){
            auto const& loader = loaders[key.second];
            stage1Queue.push(loader->clone(FileHandle::automatic(key.first)));
        }
        load();

        Logger::print_raw("Complete\n");
    }
}