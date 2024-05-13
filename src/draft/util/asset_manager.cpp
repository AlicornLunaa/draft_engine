#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

#include "draft/util/asset_manager.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    // Variables
    Assets::ResourceList Assets::resources{};
    Assets::PackageList Assets::packages{};
    Assets::AssetPackage* Assets::currentPackage = nullptr;
    std::unique_ptr<Texture> Assets::MISSING_TEXTURE = nullptr;
    std::unique_ptr<Image> Assets::MISSING_IMAGE = nullptr;
    std::unique_ptr<Model> Assets::MISSING_MODEL = nullptr;
    std::unique_ptr<Shader> Assets::MISSING_SHADER = nullptr;
    std::unique_ptr<Font> Assets::MISSING_FONT = nullptr;
    std::unique_ptr<SoundBuffer> Assets::MISSING_AUDIO = nullptr;

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

    template<>
    const Texture& Assets::get_missing_placeholder(){
        if(!MISSING_TEXTURE) MISSING_TEXTURE = std::make_unique<Texture>(FileHandle::automatic("assets/textures/missing_texture.png"));
        return *MISSING_TEXTURE;
    }

    template<>
    const Image& Assets::get_missing_placeholder(){
        if(!MISSING_IMAGE) MISSING_IMAGE = std::make_unique<Image>(FileHandle::automatic("assets/textures/missing_texture.png"));
        return *MISSING_IMAGE;
    }

    template<>
    const Model& Assets::get_missing_placeholder(){
        if(!MISSING_MODEL) MISSING_MODEL = std::make_unique<Model>(FileHandle::automatic("assets/models/missing_model.glb"));
        return *MISSING_MODEL;
    }

    template<>
    const Shader& Assets::get_missing_placeholder(){
        if(!MISSING_SHADER) MISSING_SHADER = std::make_unique<Shader>(FileHandle::automatic("assets/shaders/missing_shader/vertex.glsl"), FileHandle::automatic("assets/shaders/missing_shader/fragment.glsl"));
        return *MISSING_SHADER;
    }

    template<>
    const Font& Assets::get_missing_placeholder(){
        if(!MISSING_FONT) MISSING_FONT = std::make_unique<Font>(FileHandle::automatic("assets/fonts/missing_font.ttf"));
        return *MISSING_FONT;
    }

    template<>
    const SoundBuffer& Assets::get_missing_placeholder(){
        if(!MISSING_AUDIO) MISSING_AUDIO = std::make_unique<SoundBuffer>(FileHandle::automatic("assets/audio/missing_audio.wav"));
        return *MISSING_AUDIO;
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