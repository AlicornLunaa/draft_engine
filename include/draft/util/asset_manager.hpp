#pragma once

#include "draft/audio/sound_buffer.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace Draft {
    // Owns and manages all resources inside the game, singleton to encapsulate the data
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
        typedef std::unordered_map<size_t, AssetPackage> PackageList;

        // Variables
        static ResourceList resources;
        static PackageList packages;
        static AssetPackage* currentPackage;
        static std::unique_ptr<Texture> MISSING_TEXTURE;
        static std::unique_ptr<Image> MISSING_IMAGE;
        static std::unique_ptr<Model> MISSING_MODEL;
        static std::unique_ptr<Shader> MISSING_SHADER;
        static std::unique_ptr<Font> MISSING_FONT;
        static std::unique_ptr<SoundBuffer> MISSING_AUDIO;

        // Constructors
        Assets();
        ~Assets();

        // Helper functions
        static void remove_orphans(ResourcePool& map);

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
        static const T& get_missing_placeholder();

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