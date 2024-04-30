#include <filesystem>

#include "draft/util/asset_manager.hpp"
#include "draft/rendering/model.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    // Private functions
    Texture* AssetManager::load_static_texture(const std::string& path){
        // Loads raw data from binary to a texture
        return new Texture(FileHandle(path, FileHandle::INTERNAL));
    }

    Model* AssetManager::load_static_model(const std::string& path){
        return new Model(FileHandle(path, FileHandle::INTERNAL));
    }

    Shader* AssetManager::load_static_shader(const std::string& vertex, const std::string& fragment){
        return new Shader(FileHandle(vertex, FileHandle::INTERNAL), FileHandle(fragment, FileHandle::INTERNAL));
    }

    // Constructors
    AssetManager::AssetManager(){}
    AssetManager::~AssetManager(){ clear(); }

    // Functions
    void AssetManager::clear(){
        // Clear out the manager
        textureMap.clear();
        for(auto* ptr : textureArray)
            delete ptr;
        textureArray.clear();
        
        shaderMap.clear();
        for(auto* ptr : shaderArray)
            delete ptr;
        shaderArray.clear();
    }

    void AssetManager::reload(){
        Logger::print(Level::INFO, "Asset Manager", "Reloading...");

        for(auto* res : textureArray) res->reload();
        for(auto* res : modelArray) res->reload();
        for(auto* res : shaderArray) res->reload();

        Logger::print_raw("Complete\n");
    }

    void AssetManager::load(){
        // Loads every asset, starting with textures
        while(!textureQueue.empty()){
            auto& handle = textureQueue.front();

            textureArray.push_back(new Texture(handle));
            textureMap[handle.get_path()] = (textureArray.size() - 1);

            if(!textureArray.back()->is_loaded()){
                Logger::println(Level::SEVERE, "Asset Manager", "Failed to load texture " + handle.filename());
            } else {
                Logger::println(Level::INFO, "Asset Manager", "Loaded texture " + handle.filename());
            }

            textureQueue.pop();
        }

        while(!modelQueue.empty()){
            auto& handle = modelQueue.front();

            modelArray.push_back(new Model(handle));
            modelMap[handle.get_path()] = (modelArray.size() - 1);

            Logger::println(Level::INFO, "Asset Manager", "Loaded model " + handle.filename());
            modelQueue.pop();
        }

        while(!shaderQueue.empty()){
            auto& handle = shaderQueue.front();

            if(!handle.is_directory()){
                Logger::println(Level::SEVERE, "Asset Manager", "Failed to load shader " + handle.filename());
                shaderQueue.pop();
                continue;
            }

            shaderArray.push_back(new Shader(handle));
            shaderMap[handle.get_path()] = (shaderArray.size() - 1);

            // Folder actually exists, load the optional vertex and fragment shaders
            Logger::println(Level::INFO, "Asset Manager", "Loaded shader " + handle.filename());
            
            shaderQueue.pop();
        }
    }

    const Texture& AssetManager::get_texture(const std::string& name) const {
        const auto& result = textureMap.find(name);

        if(result == textureMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find texture " + name);
            return AssetManager::get_missing_texture();
        }
        
        return *textureArray[textureMap.at(name)];
    }

    const Model& AssetManager::get_model(const std::string& name) const {
        const auto& result = modelMap.find(name);

        if(result == modelMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find model " + name);
            return AssetManager::get_missing_model();
        }
        
        return *modelArray[modelMap.at(name)];
    }

    Shader& AssetManager::get_shader(const std::string& name) const {
        const auto& result = shaderMap.find(name);

        if(result == shaderMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find shader " + name);
            return AssetManager::get_missing_shader();
        }
        
        return *shaderArray[shaderMap.at(name)];
    }


    // Static variables
    Texture* AssetManager::MISSING_TEXTURE = nullptr;
    Texture* AssetManager::EMPTY_NORMAL_MAP = nullptr;
    Texture* AssetManager::DEBUG_WHITE = nullptr;
    Texture* AssetManager::DEBUG_BLACK = nullptr;
    Model* AssetManager::MISSING_MODEL = nullptr;
    Shader* AssetManager::MISSING_SHADER = nullptr;

    // Static functions
    const Texture& AssetManager::get_missing_texture(){
        // Load on-demand
        if(!MISSING_TEXTURE) MISSING_TEXTURE = load_static_texture("assets/missing_texture.png");
        return *MISSING_TEXTURE;
    }

    const Texture& AssetManager::get_empty_normal_map(){
        // Load on-demand
        if(!EMPTY_NORMAL_MAP) EMPTY_NORMAL_MAP = load_static_texture("assets/empty_normal_map.png");
        return *EMPTY_NORMAL_MAP;
    }

    const Texture& AssetManager::get_debug_white(){
        // Load on-demand
        if(!DEBUG_WHITE) DEBUG_WHITE = load_static_texture("assets/debug_white.png");
        return *DEBUG_WHITE;
    }

    const Texture& AssetManager::get_debug_black(){
        // Load on-demand
        if(!DEBUG_BLACK) DEBUG_BLACK = load_static_texture("assets/debug_black.png");
        return *DEBUG_BLACK;
    }

    const Model& AssetManager::get_missing_model(){
        // Load on-demand
        if(!MISSING_MODEL) MISSING_MODEL = load_static_model("assets/missing_model.glb");
        return *MISSING_MODEL;
    }

    Shader& AssetManager::get_missing_shader(){
        // Load on-demand
        if(!MISSING_SHADER) MISSING_SHADER = load_static_shader("assets/missing_shader/vertex.glsl", "assets/missing_shader/fragment.glsl");
        return *MISSING_SHADER;
    }
}