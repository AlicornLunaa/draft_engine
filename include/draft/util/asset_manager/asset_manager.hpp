#pragma once

#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/file_handle.hpp"

#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Draft {
    // Owns and manages all resources inside the game, singleton to encapsulate the data
    class Assets {
    private:
        // Variables
        std::unordered_map<std::type_index, const BaseLoader*> loaderTemplates;
        std::unordered_map<std::string, std::shared_ptr<void>> resources;
        std::vector<std::function<void(void)>> reloadFunctions;

        std::queue<BaseLoader*> loadQueue; // Stage 1, no OpenGL, non-blocking
        std::queue<BaseLoader*> finishAsyncQueue; // Stage 2, context provided, blocking
        std::mutex asyncMutex;
        float loadingProgress = 1.f;
        bool loadingAsyncronously = false;

        // Helper funcs
        template<typename T>
        void load_sync_immediate(const FileHandle& handle){
            auto* loaderTemplate = loaderTemplates[typeid(T)];
            auto* loader = loaderTemplate->clone(handle);
            resources[handle.get_path()] = loader->load_sync();
            delete loader;
        }

        static void load_async_queue(std::queue<BaseLoader*>& loadQueue, std::queue<BaseLoader*>& finishQueue, size_t totalAssets, float* progress, std::mutex& mut);
        void finish_async_queue();
        bool has_asset_loaded(const std::string& str);

    public:
        // Variables
        static Assets manager;

        // Constructors
        Assets();
        ~Assets();

        // Functions
        template<typename T>
        void set_loader(BaseLoader* loader){
            if(loaderTemplates[typeid(T)])
                delete loaderTemplates[typeid(T)];

            loaderTemplates[typeid(T)] = loader;
        }

        template<typename T>
        const std::shared_ptr<T> get(const std::string& key, bool loadOnFail = false){
            // Retrieve an asset
            if(resources.find(key) == resources.end()){
                // No asset exists by this name, try loading it if load on fail is true
                if(loadOnFail){
                    load_sync_immediate<T>(FileHandle::automatic(key));
                } else {
                    return nullptr;
                }
            }

            // Get the resource pointer
            return std::static_pointer_cast<T>(resources.at(key));
        }

        template<typename T>
        void queue(const FileHandle& handle){
            // Check if it needs a loader too
            auto* loader = loaderTemplates[typeid(T)];
            assert(loader != nullptr && "Loader for asset does not exist");
            loadQueue.push(loader->clone(handle));
        }

        void load();
        void load_async();
        bool poll_async(); // Poll function on main thread
        float get_loading_progress();

        void reload();
        void cleanup();
    };
}