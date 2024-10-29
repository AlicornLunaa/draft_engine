#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/asset_manager/resource.hpp"
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
#include <utility>

/**
 * Unload system should swap pointer to the missing asset placeholder and then release the item
 *
 * If an item is requested, return a new handle but as a redirect to missing, then if a resource
 *      was loaded, it can be swapped to the correct asset.
 */

 // Asset class holds unique pointers to Resource class. Resource is a superclass that contains several
 // methods specific to loading and unloading. There can also be a container class called DynamicResource
 // which instead holds a reference to the pointer, so the resource can be changed at will.

namespace Draft {
    // Owns and manages all resources inside the game, singleton to encapsulate the data
    class Assets {
    private:
        // Variables
        typedef std::pair<std::string, std::type_index> ResourceKey;

        struct KeyHash {
            std::size_t operator()(const ResourceKey& k) const {
                return std::hash<std::string>()(k.first) ^ k.second.hash_code();
            }
        };

        std::unordered_map<std::type_index, std::unique_ptr<BaseLoader>> loaders;
        std::unordered_map<std::type_index, AssetPtr> placeholders;
        std::unordered_map<ResourceKey, AssetPtr, KeyHash> resources;

        std::queue<std::unique_ptr<BaseLoader>> stage1Queue; // Stage 1, no OpenGL, non-blocking
        std::queue<std::unique_ptr<BaseLoader>> stage2Queue; // Stage 2, context provided, blocking
        std::mutex asyncMutex;
        float loadingProgress = 1.f;
        bool loadingAsyncronously = false;

        // Helper funcs
        template<typename T>
        void load_sync_immediate(const FileHandle& handle){
            auto const& loaderTemplate = loaders[typeid(T)];
            auto loader = loaderTemplate->clone(handle);
            resources.insert_or_assign(ResourceKey{handle.get_path(), typeid(T)}, loader->load_sync());
        }

        static void load_async_queue(std::queue<std::unique_ptr<BaseLoader>>& loadQueue, std::queue<std::unique_ptr<BaseLoader>>& finishQueue, size_t totalAssets, float* progress, std::mutex& mut);
        void finish_async_queue();

        template<typename T>
        bool has_asset_loaded(const std::string& str){ return resources.find({str, typeid(T)}) != resources.end(); }

    public:
        // Variables
        static Assets manager;

        // Constructors
        Assets();
        ~Assets() = default;

        // Functions
        template<typename T>
        void register_loader(BaseLoader* pTemplate){
            loaders.insert_or_assign(typeid(T), std::unique_ptr<BaseLoader>(pTemplate));
        }

        template<typename T>
        void register_placeholder(T* placeholder){
            placeholders.insert_or_assign(typeid(T), make_asset_ptr(placeholder));
        }

        template<typename T>
        Resource<T> get(const std::string& key, bool loadOnFail = false){
            // Retrieve an asset
            ResourceKey keyPair{key, typeid(T)};

            if(resources.find(keyPair) == resources.end()){
                // No asset exists by this name, try loading it if load on fail is true
                if(loadOnFail){
                    load_sync_immediate<T>(FileHandle::automatic(key));
                } else {
                    // Create a redirect unique pointer to the placeholder asset
                    T* ptr = static_cast<T*>(placeholders.at(keyPair.second).get());
                    resources.insert({keyPair, AssetPtr(ptr, [](void* p){})});
                }
            }

            // Get the resource pointer
            return Resource<T>(resources.at(keyPair));
        }

        template<typename T>
        void queue(const FileHandle& handle){
            // Check if it needs a loader too
            auto const& loader = loaders[typeid(T)];
            assert(loader != nullptr && "Loader for asset does not exist");
            stage1Queue.push(loader->clone(handle));
        }

        void load();
        void load_async();
        bool poll_async(); // Poll function on main thread
        float get_loading_progress();

        void reload();
    };
}