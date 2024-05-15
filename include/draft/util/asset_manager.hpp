#pragma once

#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <cstddef>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Draft {
    // Owns and manages all resources inside the game, singleton to encapsulate the data
    class Assets {
    public:
        // Structures
        struct BaseLoader {
            virtual ~BaseLoader(){}
            virtual void* load_sync(const FileHandle& handle) = 0; // Loading on main thread
            virtual void* load_async(const FileHandle& handle) = 0; // Loading in a separate thread with no OpenGL context
            virtual void* finish_async_gl() = 0; // Called after async loading without OpenGL
        };

        template<typename T>
        struct GenericLoader : public BaseLoader {
            // Loading on main thread
            virtual void* load_sync(const FileHandle& handle) override {
                // Default to basic call of default filehandle constructor
                try {
                    return new T(handle);
                } catch(int e){
                    Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
                }

                return nullptr;
            }

            // Loading in a separate thread with no OpenGL context
            virtual void* load_async(const FileHandle& handle) override {
                // Default to basic call of default filehandle constructor
                try {
                    return new T(handle);
                } catch(int e){
                    Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
                }

                return nullptr;
            }

            // Create the finalized object
            virtual void* finish_async_gl() override {
                return nullptr;
            }
        };

    private:
        // Structures
        struct BaseResource {
            size_t ownerCount = 0;
            FileHandle handle;

            BaseResource(const FileHandle& handle) : handle(handle) {}
            virtual ~BaseResource(){}

            virtual bool is_loaded() = 0;
            virtual void reload() = 0;
            virtual void load_sync() = 0;
            virtual void load_async() = 0;
            virtual void finish_async() = 0;
        };

        template<typename T>
        struct Resource : public BaseResource {
            T* resPtr = nullptr;
            BaseLoader* loader = nullptr;
            
            Resource(T* ptr) : resPtr(ptr), loader(nullptr) {}
            Resource(BaseLoader* loader, const FileHandle& handle) : resPtr(nullptr), loader(loader), BaseResource(handle) {}
            Resource(const Resource<T>& other) = delete;
            virtual ~Resource(){ if(resPtr) delete resPtr; }

            Resource<T>& operator=(const Resource<T>& other) = delete;

            virtual bool is_loaded() override { return !resPtr; }
            virtual void reload() override { if(resPtr) resPtr->reload(); }
            virtual void load_sync() override { resPtr = (T*)loader->load_sync(handle); }
            virtual void load_async() override { resPtr = (T*)loader->load_async(handle); }
            virtual void finish_async() override { loader->finish_async_gl(); }
        };

        struct Package {
        private:
            std::vector<BaseResource*> claimedResources;
            size_t assetCount = 0;

        public:
            const size_t id;

            Package(size_t id) : id(id){}
            Package(const Package& other) = delete;
            Package& operator=(const Package& other) = delete;

            ~Package(){
                for(auto& res : claimedResources){
                    if(!res) continue;
                    res->ownerCount--;
                }
            }

            void own(BaseResource& res){
                // Adds this package as an owner for the resource
                claimedResources.push_back(&res);
                res.ownerCount++;
                assetCount++;
            }

            inline size_t get_asset_count() const { return assetCount; }
        };

        // Variables
        static std::unordered_map<std::string, BaseResource*> resources;
        static std::unordered_map<std::type_index, BaseLoader*> loaders;
        static std::unordered_map<size_t, Package> packages;
        static Package* currentPackage;

        static std::unordered_map<std::type_index, std::vector<BaseResource*>> loadQueue;
        static std::mutex asyncMutex;
        static float loadingProgress;

        // Constructors
        Assets();
        ~Assets();

        // Helper funcs
        template<typename T>
        static void load_sync_immediate(const FileHandle& handle){
            if(!loaders[typeid(T)])
                loaders[typeid(T)] = new GenericLoader<T>();

            if(!currentPackage)
                start_package();

            resources[handle.get_path()] = new Resource<T>(loaders[typeid(T)], handle);
            resources[handle.get_path()]->load_sync();
            currentPackage->own(*resources[handle.get_path()]);
        }

        static void remove_orphans();
        static void load_async_queue(size_t totalAssets);
        static bool has_asset_loaded(const std::string& str);

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
            std::string str = handle.get_path();

            if(resources.find(str) == resources.end()){
                // No asset exists by this name, try loading it if load on fail is true
                if(loadOnFail){
                    load_sync_immediate<T>(handle);
                } else {
                    return nullptr;
                }
            }

            // Get the resource pointer
            BaseResource* rawResource = resources.at(str);
            Resource<T>* dataPtr = dynamic_cast<Resource<T>*>(rawResource);

            // Error check
            if(!dataPtr){
                Logger::println(Level::CRITICAL, "Asset Manager", "Something went seriously wrong!");
                exit(0);
            }

            return dataPtr->resPtr;
        }

        template<typename T>
        static void queue(const FileHandle& handle){
            // Check if it needs a loader too
            if(!loaders[typeid(T)])
                loaders[typeid(T)] = new GenericLoader<T>();

            loadQueue[typeid(T)].push_back(new Resource<T>(loaders[typeid(T)], handle));
        }

        static void load();
        static void load_async();
        static bool is_loading_finished();
        static float get_loading_progress();

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