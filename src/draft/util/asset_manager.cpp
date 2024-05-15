#include <cassert>
#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "draft/util/asset_manager.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    // Variables
    std::unordered_map<std::string, Assets::BaseResource*> Assets::resources{};
    std::unordered_map<std::type_index, Assets::BaseLoader*> Assets::loaders{};
    std::unordered_map<size_t, Assets::Package> Assets::packages{};
    Assets::Package* Assets::currentPackage = nullptr;

    std::unordered_map<std::type_index, std::vector<Assets::BaseResource*>> Assets::loadQueue{};
    std::mutex Assets::asyncMutex{};
    float Assets::loadingProgress = 1.f;

    // Private functions
    void Assets::remove_orphans(){
        std::vector<std::string> markedForDeletion;

        for(auto& [key, res] : resources){
            if(res->ownerCount == 0){
                // Mark for deletion
                markedForDeletion.push_back(key);
            }
        }

        for(auto& key : markedForDeletion){
            resources.erase(key);
        }
    }

    void Assets::load_async_queue(size_t totalAssets){
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
            for(auto* res : vec){
                // Implement into the loaded resources
                res->load_async();
                resources[res->handle.get_path()] = res;
                currentPackage->own(*res);

                // Try queuing up the resources that need finalized

                // Update percentage
                {
                    std::lock_guard<std::mutex> guard(asyncMutex);
                    currentLoaded++;
                    loadingProgress = currentLoaded / (float)totalAssets;
                }
            }
        }
    }

    bool Assets::has_asset_loaded(const std::string& str){
        return resources.find(str) != resources.end();
    }

    // Constructors
    Assets::Assets(){}
    Assets::~Assets(){ cleanup(); }

    // Functions
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
        remove_orphans();
    }

    void Assets::end_package(){
        assert(currentPackage && "Cannot end a package with none selected");
        end_package(currentPackage->id);
    }

    void Assets::load(){
        // If no package currently exists, start one
        if(!currentPackage)
            start_package();

        // Load everything in the load queue
        for(auto& [type, vec] : loadQueue){
            for(auto* res : vec){
                // Implement into the loaded resources
                res->load_sync();
                resources[res->handle.get_path()] = res;
                currentPackage->own(*res);
            }
        }

        // Clear queue
        loadQueue.clear();
    }

    void Assets::load_async(){
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

    bool Assets::is_loading_finished(){
        return get_loading_progress() >= 1.f;
    }

    float Assets::get_loading_progress(){
        std::lock_guard<std::mutex> guard(asyncMutex);
        return loadingProgress;
    }

    void Assets::reload(){
        Logger::print(Level::INFO, "Asset Manager", "Reloading...");

        for(auto& [key, res] : resources){
            res->reload();
        }

        Logger::print_raw("Complete\n");
    }

    void Assets::cleanup(){
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
}