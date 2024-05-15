#include <cassert>
#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "draft/util/asset_manager.hpp"
#include "draft/rendering/image.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    // Variables
    std::unordered_map<std::type_index, std::unordered_map<std::string, Assets2::Resource>> Assets2::resources{};
    std::unordered_map<std::type_index, std::vector<FileHandle>> Assets2::loadQueue{};
    std::unordered_map<std::type_index, Assets2::BaseLoader*> Assets2::loaders{};
    std::unordered_map<size_t, Assets2::Package> Assets2::packages{};
    Assets2::Package* Assets2::currentPackage = nullptr;

    std::mutex Assets2::asyncMutex{};
    float Assets2::loadingProgress = 1.f;

    // Private functions
    void Assets2::remove_orphans(){
        std::unordered_map<std::type_index, std::vector<std::string>> markedForDeletion;

        for(auto& [type, map] : resources){
            for(auto& [key, res] : map){
                if(res.ownerCount == 0){
                    // Mark for deletion
                    markedForDeletion[type].push_back(key);
                }
            }
        }

        for(auto& [type, vec] : markedForDeletion){
            for(auto& key : vec){
                resources.at(type).erase(key);
            }
        }
    }

    void Assets2::load_async_queue(size_t totalAssets){
        // Variables
        size_t currentLoaded = 0;

        // Error handle
        if(loadQueue.empty()){
            // No queue, end this thread as a success
            loadingProgress = 1.f;
            return;
        }

        // Load the assets one by one
        for(auto& [type, vec] : loadQueue){
            for(auto& handle : vec){
                // Check if it is already loaded, and skip if it does
                if(has_asset_loaded(type, handle.get_path())) continue;

                // Load the actual data
                auto& loader = loaders[type];

                // Implement into the loaded resources
                resources[type].emplace(handle.get_path(), loader->load_async(handle));
                currentPackage->own(type, resources[type].at(handle.get_path()));

                // Update percentage
                {
                    std::lock_guard<std::mutex> guard(asyncMutex);
                    currentLoaded++;
                    loadingProgress = currentLoaded / (float)totalAssets;
                }
            }
        }
    }

    bool Assets2::has_asset_loaded(const std::type_index& type, const std::string& str){
        auto& map = resources[type];
        return map.find(str) != map.end();
    }

    // Constructors
    Assets2::Assets2(){}
    Assets2::~Assets2(){ cleanup(); }

    // Functions
    size_t Assets2::start_package(){
        // Create a new package
        size_t id = packages.size();
        packages.emplace(id, id);
        currentPackage = &packages.at(id);
        return id;
    }

    void Assets2::select_package(size_t package){
        // Check if package exists
        assert(packages.find(package) != packages.end() && "Package selected does not exist");
        currentPackage = &packages.at(package);
    }

    void Assets2::end_package(size_t package){
        auto iter = packages.find(package);
        assert(iter != packages.end() && "Package ended does not exist");

        // Remove the package
        packages.erase(iter);

        // Remove pointer if deleting this package
        if(currentPackage && package == currentPackage->id)
            currentPackage = nullptr;

        // Check each resource to see if orphaned resources are ready to be deleted
        remove_orphans();
    }

    void Assets2::end_package(){
        assert(currentPackage && "Cannot end a package with none selected");
        end_package(currentPackage->id);
    }

    void Assets2::load(){
        // If no package currently exists, start one
        if(!currentPackage)
            start_package();

        // Load everything in the load queue
        for(auto& [type, vec] : loadQueue){
            for(auto& handle : vec){
                // Check if it is already loaded, and skip if it does
                if(has_asset_loaded(type, handle.get_path())) continue;

                // Load the actual data
                auto& loader = loaders[type];

                // Implement into the loaded resources
                resources[type].emplace(handle.get_path(), loader->load_sync(handle));
                currentPackage->own(type, resources[type].at(handle.get_path()));
            }
        }
    }

    void Assets2::load_async(){
        // If no package currently exists, start one
        if(!currentPackage)
            start_package();

        // Get total assets to keep a percentage
        size_t totalAssets = 0;
        loadingProgress = 0.f;

        for(auto& [type, vec] : loadQueue){
            totalAssets += vec.size();
        }

        // Spawn thread to run the asyncronous load
        std::thread loadThread(load_async_queue, totalAssets);
        loadThread.detach();
    }

    bool Assets2::is_loading_finished(){
        return get_loading_progress() >= 1.f;
    }

    float Assets2::get_loading_progress(){
        std::lock_guard<std::mutex> guard(asyncMutex);
        return loadingProgress;
    }

    void Assets2::reload(){
        Logger::print(Level::INFO, "Asset Manager", "Reloading...");

        for(auto& [type, map] : resources){
            for(auto& [str, res] : map){
                // res.ptr->reload();
            }
        }

        Logger::print_raw("Complete\n");
    }

    void Assets2::cleanup(){
        // Cleans all resources
        for(auto& [type, loader] : loaders){
            if(!loader) continue;
            delete loader;
        }

        packages.clear();
        loadQueue.clear();
        loaders.clear();
        currentPackage = nullptr;
        remove_orphans();
    }






    // Variables
    Assets::ResourceList Assets::resources{};
    Assets::MissingAssetPool Assets::missingResources{};
    Assets::PackageList Assets::packages{};
    Assets::AssetPackage* Assets::currentPackage = nullptr;
    
    std::mutex Assets::asyncMutex;
    Assets::AsyncQueue Assets::asyncQueue{};
    float Assets::loadingProgress = 1.f;

    // Helper functions
    void Assets::remove_orphans(ResourcePool& map){
        std::vector<std::string> deleteList;

        for(auto& [key, res] : map){
            if(res->package_owner_count <= 0){
                // Mark for deletion
                deleteList.push_back(key);
            }
        }

        for(auto& key : deleteList){
            delete map[key];
            map.erase(key);
        }
    }

    void Assets::load_async_queue(size_t totalAssets){
        // Variables
        size_t currentLoaded = 0;

        // Error handle
        if(asyncQueue.empty()){
            // No queue, end this thread as a success
            loadingProgress = 1.f;
            return;
        }

        // Load the assets one by one
        for(auto& [type, vec] : asyncQueue){
            for(auto& handle : vec){
                // Load raw data of the object first
                get_asset<Image>(handle);

                {
                    std::lock_guard<std::mutex> guard(asyncMutex);
                    currentLoaded++;
                    loadingProgress = currentLoaded / (float)totalAssets;
                }
            }
        }
    }

    // Constructors
    Assets::Assets(){}
    Assets::~Assets(){ cleanup(); }

    // Function interface
    size_t Assets::start_package(){
        // Create a new package
        size_t id = packages.size();
        packages.emplace(id, id);
        currentPackage = &packages.at(id);
        return id;
    }

    void Assets::select_package(size_t package){
        // Check if package exists
        assert(packages.find(package) != packages.end() && "Package selected does not exist");
        currentPackage = &packages.at(package);
    }

    void Assets::end_package(size_t package){
        auto iter = packages.find(package);
        assert(iter != packages.end() && "Package ended does not exist");

        // Remove the package
        packages.erase(iter);

        // Remove pointer if deleting this package
        if(currentPackage && package == currentPackage->id)
            currentPackage = nullptr;

        // Check each resource to see if orphaned resources are ready to be deleted
        for(auto& [type, arr] : resources){
            remove_orphans(arr);
        }
    }

    void Assets::end_package(){
        assert(currentPackage && "Cannot end a package with none selected");
        end_package(currentPackage->id);
    }

    void Assets::commit_async_load(){
        // Get total assets to keep a percentage
        size_t totalAssets = 0;
        loadingProgress = 0.f;

        for(auto& [type, vec] : asyncQueue){
            totalAssets += vec.size();
        }

        // Spawn thread to run the asyncronous load
        std::thread loadThread(load_async_queue, totalAssets);
        loadThread.detach();
    }

    bool Assets::is_loading(){
        std::lock_guard<std::mutex> guard(asyncMutex);
        return loadingProgress < 1.f;
    }

    float Assets::progress(){
        std::lock_guard<std::mutex> guard(asyncMutex);
        return loadingProgress;
    }

    void Assets::reload(){
        Logger::print(Level::INFO, "Asset Manager", "Reloading...");

        for(auto& [type, arr] : resources){
            for(auto& [str, res] : arr){
                res->reload();
            }
        }

        Logger::print_raw("Complete\n");
    }

    void Assets::cleanup(){
        // Cleans all resources
        packages.clear();
        currentPackage = nullptr;

        for(auto& [type, arr] : resources){
            remove_orphans(arr);
        }
    }
}