#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

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
    void Assets::load_async_queue(std::queue<BaseLoader*>& loadQueue, std::queue<BaseLoader*>& finishQueue, size_t totalAssets, float* progress, std::mutex& mut){
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
            auto* loader = loadQueue.front();
            loader->load_async();

            {
                // Prevent race conditions
                std::lock_guard<std::mutex> guard(mut);

                // Try queuing up the resources that need finalized
                finishQueue.push(loader);

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

        while(!finishAsyncQueue.empty()){
            auto* loader = finishAsyncQueue.front();
            resources[loader->handle.get_path()] = loader->finish_async_gl();
            delete loader;
            finishAsyncQueue.pop();
        }
    }

    bool Assets::has_asset_loaded(const std::string& str){
        return resources.find(str) != resources.end();
    }

    // Constructors
    Assets::Assets(){
        // Setup loaders
        set_loader<Image>(new GenericLoader<Image>());
        set_loader<Font>(new GenericLoader<Font>());
        set_loader<SoundBuffer>(new GenericSyncLoader<SoundBuffer>());
        set_loader<Model>(new GenericSyncLoader<Model>());
        set_loader<Shader>(new GenericSyncLoader<Shader>());
        set_loader<Texture>(new GenericSyncLoader<Texture>());
        set_loader<ParticleProps>(new ParticleLoader());
        set_loader<nlohmann::json>(new JSONLoader());
    }

    Assets::~Assets(){
        cleanup();

        for(auto& [type, ptr] : loaderTemplates){
            delete ptr;
        }

        loaderTemplates.clear();
    }

    // Public vars
    Assets Assets::manager;

    // Functions
    void Assets::load(){
        // Load everything in the load queue
        while(!loadQueue.empty()){
            auto* loader = loadQueue.front();
            resources[loader->handle.get_path()] = loader->load_sync();
            delete loader;
            loadQueue.pop();
        }
    }

    void Assets::load_async(){
        // Get total assets to keep a percentage
        size_t totalAssets = 0;
        loadingProgress = 0.f;
        loadingAsyncronously = true;
        totalAssets += loadQueue.size();

        // Spawn thread to run the asyncronous load
        std::thread loadThread([this, totalAssets](){ load_async_queue(loadQueue, finishAsyncQueue, totalAssets, &loadingProgress, asyncMutex); });
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

        for(auto& func : reloadFunctions){
            func();
        }

        Logger::print_raw("Complete\n");
    }

    void Assets::cleanup(){
        // Cleans all resources
        while(!loadQueue.empty()){
            delete loadQueue.front();
            loadQueue.pop();
        }

        while(!finishAsyncQueue.empty()){
            delete finishAsyncQueue.front();
            finishAsyncQueue.pop();
        }

        reloadFunctions.clear();
        resources.clear();
    }
}