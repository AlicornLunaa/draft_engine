#pragma once

#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <any>
#include <cassert>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Draft {
    // Owns and manages all resources inside the game, singleton to encapsulate the data
    class Assets2 {
    public:
        // Structures
        struct BaseLoader {
            virtual ~BaseLoader(){}
            virtual std::any load_sync(const FileHandle& handle) = 0; // Loading on main thread
            virtual std::any load_async(const FileHandle& handle) = 0; // Loading in a separate thread with no OpenGL context
        };

        template<typename T>
        struct GenericLoader : public BaseLoader {
            // Loading on main thread
            virtual std::any load_sync(const FileHandle& handle) override {
                // Default to basic call of default filehandle constructor
                try {
                    return T(handle);
                } catch(int e){
                    Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
                }

                return nullptr;
            }

            // Loading in a separate thread with no OpenGL context
            virtual std::any load_async(const FileHandle& handle) override {
                return load_sync(handle);
            }
        };

    private:
        // Structures
        struct Resource {
            std::any resource;
            size_t ownerCount = 0;

            template<typename ...Args>
            Resource(const std::any& res) : resource(res) {}
            Resource(const Resource& other) = delete;
            Resource& operator=(const Resource& other) = delete;

            template<typename U>
            U* get_ptr(){ return std::any_cast<U>(&resource); }
        };

        struct Package {
        private:
            std::unordered_map<std::type_index, std::vector<Resource*>> claimedResources;
            size_t assetCount = 0;

        public:
            const size_t id;

            Package(size_t id) : id(id){}
            Package(const Package& other) = delete;
            Package& operator=(const Package& other) = delete;

            ~Package(){
                for(auto& [type, vec] : claimedResources){
                    for(auto* res : vec){
                        if(!res) continue;
                        res->ownerCount--;
                    }
                }
            }

            void own(std::type_index type, Resource& res){
                // Adds this package as an owner for the resource
                claimedResources[type].push_back(&res);
                res.ownerCount++;
                assetCount++;
            }

            inline size_t get_asset_count() const { return assetCount; }
        };

        // Variables
        static std::unordered_map<std::type_index, std::unordered_map<std::string, Resource>> resources;
        static std::unordered_map<std::type_index, std::vector<FileHandle>> loadQueue;
        static std::unordered_map<std::type_index, BaseLoader*> loaders;
        static std::unordered_map<size_t, Package> packages;
        static Package* currentPackage;

        static std::mutex asyncMutex;
        static float loadingProgress;

        // Constructors
        Assets2();
        ~Assets2();

        // Helper funcs
        static void remove_orphans();
        static void load_async_queue(size_t totalAssets);
        static bool has_asset_loaded(const std::type_index& type, const std::string& str);

    public:
        // Functions
        static size_t start_package();
        static void select_package(size_t package);
        static void end_package(size_t package);
        static void end_package();

        template<typename T>
        static void set_loader(BaseLoader* loader){
            loaders[typeid(T)] = loader;
        }

        template<typename T>
        static const T* get(const FileHandle& handle, bool loadOnFail = false){
            // Retrieve an asset
            auto& resourceMap = resources[typeid(T)];
            std::string str = handle.get_path();

            if(resourceMap.find(str) == resourceMap.end()){
                // No asset exists by this name, try loading it if load on fail is true
                if(loadOnFail){
                    queue<T>(handle);
                    load();
                } else {
                    return nullptr;
                }
            }

            // Get the resource pointer
            Resource& rawResource = resourceMap.at(str);
            T* dataPtr = rawResource.get_ptr<T>();

            // Error check
            if(!dataPtr){
                Logger::println(Level::CRITICAL, "Asset Manager", "Something went seriously wrong!");
                exit(0);
            }

            return dataPtr;
        }

        template<typename T>
        static void queue(const FileHandle& handle){
            // Check if it needs a loader too
            if(!loaders[typeid(T)])
                loaders[typeid(T)] = new GenericLoader<T>();

            loadQueue[typeid(T)].push_back(handle);
        }

        static void load();
        static void load_async();
        static bool is_loading_finished();
        static float get_loading_progress();

        static void reload();
        static void cleanup();
    };

    class Assets {
    private:
        // Structures
        struct UnnamedResource {
            size_t package_owner_count = 0;
            virtual ~UnnamedResource(){}
            virtual void reload() = 0;
        };

        template<typename T>
        struct Resource : public UnnamedResource {
            std::unique_ptr<T> ptr;

            template<typename ...Args>
            Resource(Args&&... args) : ptr(std::make_unique<T>(args...)) {}

            virtual void reload(){
                ptr->reload();
            }
        };

        struct AssetPackage {
        private:
            std::unordered_map<std::type_index, std::vector<UnnamedResource*>> resourcePtrs{}; // Data type to resource unique pointers
            size_t assetCount = 0;

        public:
            const size_t id;

            AssetPackage(size_t id) : id(id) {}
            AssetPackage(const AssetPackage& other) = delete;
            AssetPackage& operator=(const AssetPackage& other) = delete;
            ~AssetPackage(){
                for(auto& [type, arr] : resourcePtrs){
                    for(auto res : arr){
                        res->package_owner_count--;
                    }
                }
            }

            inline size_t get_asset_count() const { return assetCount; }

            template<typename T>
            void own(UnnamedResource* res){
                // Adds this package as an owner for the resource
                auto& vec = resourcePtrs[typeid(T)];
                vec.push_back(res);
                assetCount++;
                res->package_owner_count++;
            }
        };

        // Type aliases
        typedef std::unordered_map<std::string, UnnamedResource*> ResourcePool;
        typedef std::unordered_map<std::type_index, ResourcePool> ResourceList;
        typedef std::unordered_map<std::type_index, std::unique_ptr<std::any>> MissingAssetPool;
        typedef std::unordered_map<std::type_index, std::vector<FileHandle>> AsyncQueue;
        typedef std::unordered_map<size_t, AssetPackage> PackageList;

        // Variables
        static ResourceList resources;
        static MissingAssetPool missingResources;
        static PackageList packages;
        static AssetPackage* currentPackage;

        static std::mutex asyncMutex;
        static AsyncQueue asyncQueue;
        static float loadingProgress;

        // Constructors
        Assets();
        ~Assets();

        // Helper functions
        static void remove_orphans(ResourcePool& map);
        static void load_async_queue(size_t totalAssets);

    public:
        // Functions
        static size_t start_package();
        static void select_package(size_t package);
        static void end_package(size_t package);
        static void end_package();

        template<typename T>
        static const T& get_asset(const FileHandle& handle){
            // If no package currently exists, start one
            if(!currentPackage)
                start_package();

            // Load or retrieve an external asset
            std::string str = handle.get_path();
            ResourcePool& resourcePool = resources[typeid(T)];

            if(resourcePool.find(str) == resourcePool.end()){
                // No asset exists by this name, try loading it
                try {
                    resourcePool.emplace(str, new Resource<T>(handle));
                } catch(int e){
                    Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
                    return get_missing_placeholder<T>();
                }

                currentPackage->own<T>(resourcePool.at(str));
            }

            // Cast to the derived class for the named resource
            UnnamedResource* resource = resourcePool.at(str);
            Resource<T>* namedResource = dynamic_cast<Resource<T>*>(resource);

            // Error check
            if(!namedResource){
                Logger::println(Level::CRITICAL, "Asset Manager", "Something went seriously wrong!");
                exit(0);
            }

            return *namedResource->ptr;
        }

        template<typename T>
        static const T& get_missing_placeholder(){
            assert(missingResources[typeid(T)] && "Asset is missing a placeholder");
            return *std::any_cast<T*>(&missingResources[typeid(T)]);
        }

        template<typename T, typename... Args>
        static void set_missing_placeholder(const Args& ...args){
            // missingResources.insert(std::make_pair(typeid(T), std::make_unique<T>(args...)));
        }

        template<typename T>
        static void queue_async_load(const FileHandle& handle){
            // Creates an asyncronous loading queue
            asyncQueue[typeid(T)].push_back(handle);
        }

        static void commit_async_load();
        static bool is_loading();
        static float progress();

        static void reload();
        static void cleanup();
    };

    class AssetManager {
    private:
        size_t packageID;

    public:
        AssetManager() : packageID(Assets::start_package()) {}
        AssetManager(const AssetManager& other) = delete;
        AssetManager& operator=(const AssetManager& other) = delete;
        ~AssetManager(){ Assets::end_package(packageID); }
    };
}