#include <filesystem>

#include "draft/util/asset_manager.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    // Constructors
    AssetManager::AssetManager(){}
    AssetManager::~AssetManager(){ clear(); }

    // Functions
    void AssetManager::clear(){
        // Clear out the manager
        textureQueue.clear();
        textureMap.clear();
        textureArray.clear();
        
        shaderQueue.clear();
        shaderMap.clear();
        for(auto* ptr : shaderArray)
            delete ptr;
        shaderArray.clear();

        // audioQueue.clear();
        // audioMap.clear();
        // for(auto* ptr : audioArray)
        //     delete ptr;
        // audioArray.clear();

        // fontQueue.clear();
        // fontMap.clear();
        // fontArray.clear();
    }

    void AssetManager::reload(){
        Logger::println(Level::INFO, "Asset Manager", "Reloading...");

        for(size_t i = 0; i < textureArray.size(); i++){
            textureArray[i]->reload();
        }

        for(size_t i = 0; i < shaderArray.size(); i++){
            shaderArray[i]->reload();
        }

        // for(size_t i = 0; i < audioQueue.size(); i++){
        //     const std::string& path = audioQueue[i];
        //     loadAudio(audioArray[i], path);
        // }

        // for(size_t i = 0; i < fontQueue.size(); i++){
        //     const std::string& path = fontQueue[i];
        //     loadFont(fontArray[i], path);
        // }
    }

    void AssetManager::load(){
        // Loads every asset, starting with textures
        for(const std::string& path : textureQueue){
            textureArray.push_back(new Texture(path));
            textureMap[path] = (textureArray.size() - 1);
            loadTexture(textureArray.back(), path);
        }

        for(const std::string& path : shaderQueue){
            shaderArray.push_back(new Shader(path));
            shaderMap[path] = (shaderArray.size() - 1);
            loadShader(shaderArray.back(), path);
        }

        // for(const std::string& path : audioQueue){
        //     audioArray.push_back(new sf::SoundBuffer());
        //     audioMap[path] = (audioArray.size() - 1);
        //     loadAudio(audioArray.back(), path);
        // }

        // for(const std::string& path : fontQueue){
        //     fontArray.push_back(sf::Font());
        //     fontMap[path] = (fontArray.size() - 1);
        //     loadFont(fontArray.back(), path);
        // }
    }

    void AssetManager::queueTexture(const std::string& path){
        textureQueue.push_back(path);
    }

    void AssetManager::queueShader(const std::string& path) {
        shaderQueue.push_back(path);
    }

    // void AssetManager::queueAudio(const std::string& path) {
    //     audioQueue.push_back(path);
    // }

    // void AssetManager::queueFont(const std::string& path) {
    //     fontQueue.push_back(path);
    // }

    Texture& AssetManager::getTexture(const std::string& name) const {
        const auto& result = textureMap.find(name);

        if(result == textureMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find texture " + name);
            return *textureArray[0];
        }
        
        return *textureArray[textureMap.at(name)];
    }

    Shader& AssetManager::getShader(const std::string& name) const {
        const auto& result = shaderMap.find(name);

        if(result == shaderMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find shader " + name);
            return *shaderArray[0];
        }
        
        return *shaderArray[shaderMap.at(name)];
    }

    // const sf::SoundBuffer& AssetManager::getAudio(const std::string& name) const {
    //     const auto& result = audioMap.find(name);

    //     if(result == audioMap.end()){
    //         Logger::println(Level::SEVERE, "Asset Manager", "Failed to find audio " + name);
    //         return *audioArray[0];
    //     }
        
    //     return *audioArray[audioMap.at(name)];
    // }

    // const sf::Font& AssetManager::getFont(const std::string& name) const {
    //     const auto& result = fontMap.find(name);

    //     if(result == fontMap.end()){
    //         Logger::println(Level::SEVERE, "Asset Manager", "Failed to find font " + name);
    //         return fontArray[0];
    //     }
        
    //     return fontArray[fontMap.at(name)];
    // }

    // Private functionsz
    void AssetManager::loadTexture(Texture* texture, const std::string& path){
        if(!texture->is_loaded()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to load texture " + path);
        } else {
            Logger::println(Level::INFO, "Asset Manager", "Loaded texture " + path);
        }
    }

    void AssetManager::loadShader(Shader* shader, const std::string& path){
        // Check if the main folder exists
        if(!std::filesystem::is_directory(path)){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to load shader " + path);
        } else {
            // Folder actually exists, load the optional vertex and fragment shaders
            Logger::println(Level::INFO, "Asset Manager", "Loaded shader " + path);
        }
    }

    // void AssetManager::loadAudio(sf::SoundBuffer* audio, const std::string& path){
    //     if(!audio->loadFromFile(path)){
    //         Logger::println(Level::SEVERE, "Asset Manager", "Failed to load audio " + path);
    //     } else {
    //         Logger::println(Level::INFO, "Asset Manager", "Loaded audio " + path);
    //     }
    // }

    // void AssetManager::loadFont(sf::Font& font, const std::string& path){
    //     if(!font.loadFromFile(path)){
    //         Logger::println(Level::SEVERE, "Asset Manager", "Failed to load font " + path);
    //     } else {
    //         Logger::println(Level::INFO, "Asset Manager", "Loaded font " + path);
    //     }
    // }
}