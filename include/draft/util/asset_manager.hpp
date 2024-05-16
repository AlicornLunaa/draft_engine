#pragma once

#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <cassert>
#include <cstddef>
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
    public:
        // Structures
        struct BaseLoader {
            FileHandle handle;
            virtual ~BaseLoader(){}

            virtual std::shared_ptr<void> load_sync() const = 0; // Loading on main thread

            virtual void load_async() = 0; // Loading in a separate thread with no OpenGL context
            virtual std::shared_ptr<void> finish_async_gl() = 0; // Called after async loading without OpenGL

            virtual BaseLoader* clone(const FileHandle& handle) const = 0;
        };

        template<typename T>
        struct GenericLoader : public BaseLoader {
            // Variables
            std::vector<std::byte> rawData;

            // Loading on main thread
            virtual std::shared_ptr<void> load_sync() const override {
                // Default to basic call of default filehandle constructor
                try {
                    return std::shared_ptr<T>(new T(handle), [](void* ptr){ delete static_cast<T*>(ptr); });
                } catch(int e){
                    Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
                }

                return nullptr;
            }

            // Loading in a separate thread with no OpenGL context
            virtual void load_async() override {
                rawData = handle.read_bytes();
            }

            // Create the finalized object
            virtual std::shared_ptr<void> finish_async_gl() override {
                // Default to basic call of default filehandle constructor
                try {
                    return std::shared_ptr<T>(new T(rawData), [](void* ptr){ delete static_cast<T*>(ptr); });
                } catch(int e){
                    Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
                }

                return nullptr;
            }

            // Cloning
            virtual BaseLoader* clone(const FileHandle& handle) const override {
                auto* ptr = new GenericLoader<T>();
                ptr->handle = handle;
                return ptr;
            }
        };

        template<typename T>
        struct GenericSyncLoader : public BaseLoader {
            // Loading on main thread
            virtual std::shared_ptr<void> load_sync() const override {
                // Default to basic call of default filehandle constructor
                try {
                    return std::shared_ptr<T>(new T(handle), [](void* ptr){ delete static_cast<T*>(ptr); });
                } catch(int e){
                    Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
                }

                return nullptr;
            }

            // Loading in a separate thread with no OpenGL context
            virtual void load_async() override {}

            // Create the finalized object
            virtual std::shared_ptr<void> finish_async_gl() override {
                // Default to basic call of default filehandle constructor
                try {
                    return std::shared_ptr<T>(new T(handle), [](void* ptr){ delete static_cast<T*>(ptr); });
                } catch(int e){
                    Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
                }

                return nullptr;
            }

            // Cloning
            virtual BaseLoader* clone(const FileHandle& handle) const override {
                auto* ptr = new GenericSyncLoader<T>();
                ptr->handle = handle;
                return ptr;
            }
        };

    private:
        // Variables
        std::unordered_map<std::type_index, const BaseLoader*> loaderTemplates;
        std::unordered_map<std::string, std::shared_ptr<void>> resources;

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