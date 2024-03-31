#include <filesystem>

#include "draft/util/asset_manager.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    // Constructors
    AssetManager::AssetManager(){}

    AssetManager::~AssetManager(){
        clear();
    }

    // Functions
    void AssetManager::clear(){
        // Clear out the manager
        textureQueue.clear();
        textureMap.clear();
        textureArray.clear();

        audioQueue.clear();
        audioMap.clear();
        for(auto* ptr : audioArray)
            delete ptr;
        audioArray.clear();
        
        shaderQueue.clear();
        shaderMap.clear();
        for(auto* ptr : shaderArray)
            delete ptr;
        shaderArray.clear();

        fontQueue.clear();
        fontMap.clear();
        fontArray.clear();
    }

    void AssetManager::reload(){
        Logger::println(Level::INFO, "Asset Manager", "Reloading...");

        for(size_t i = 0; i < textureQueue.size(); i++){
            const std::string& path = textureQueue[i];
            loadTexture(textureArray[i], path);
        }

        for(size_t i = 0; i < audioQueue.size(); i++){
            const std::string& path = audioQueue[i];
            loadAudio(audioArray[i], path);
        }

        for(size_t i = 0; i < shaderQueue.size(); i++){
            const std::string& path = shaderQueue[i];
            loadShader(shaderArray[i], path);
        }

        for(size_t i = 0; i < fontQueue.size(); i++){
            const std::string& path = fontQueue[i];
            loadFont(fontArray[i], path);
        }
    }

    void AssetManager::load(){
        // Loads every asset, starting with textures
        for(const std::string& path : textureQueue){
            textureArray.push_back(sf::Texture());
            textureMap[path] = (textureArray.size() - 1);
            loadTexture(textureArray.back(), path);
        }

        for(const std::string& path : audioQueue){
            audioArray.push_back(new sf::SoundBuffer());
            audioMap[path] = (audioArray.size() - 1);
            loadAudio(audioArray.back(), path);
        }

        for(const std::string& path : shaderQueue){
            shaderArray.push_back(new sf::Shader());
            shaderMap[path] = (shaderArray.size() - 1);
            loadShader(shaderArray.back(), path);
        }

        for(const std::string& path : fontQueue){
            fontArray.push_back(sf::Font());
            fontMap[path] = (fontArray.size() - 1);
            loadFont(fontArray.back(), path);
        }
    }

    void AssetManager::queueTexture(const std::string& path){
        textureQueue.push_back(path);
    }

    void AssetManager::queueAudio(const std::string& path) {
        audioQueue.push_back(path);
    }

    void AssetManager::queueShader(const std::string& path) {
        shaderQueue.push_back(path);
    }

    void AssetManager::queueFont(const std::string& path) {
        fontQueue.push_back(path);
    }

    sf::Texture& AssetManager::getTexture(const std::string& name) const {
        const auto& result = textureMap.find(name);

        if(result == textureMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find texture " + name);
            return const_cast<sf::Texture&>(textureArray[0]);
        }
        
        // I know, evil scary const_cast but I really didnt wanna fight with the array
        return const_cast<sf::Texture&>(textureArray[textureMap.at(name)]);
    }

    const sf::SoundBuffer& AssetManager::getAudio(const std::string& name) const {
        const auto& result = audioMap.find(name);

        if(result == audioMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find audio " + name);
            return *audioArray[0];
        }
        
        return *audioArray[audioMap.at(name)];
    }

    sf::Shader& AssetManager::getShader(const std::string& name) const {
        const auto& result = shaderMap.find(name);

        if(result == shaderMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find shader " + name);
            return *shaderArray[0];
        }
        
        return *shaderArray[shaderMap.at(name)];
    }

    const sf::Font& AssetManager::getFont(const std::string& name) const {
        const auto& result = fontMap.find(name);

        if(result == fontMap.end()){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to find font " + name);
            return fontArray[0];
        }
        
        return fontArray[fontMap.at(name)];
    }

    // Private functionsz
    void AssetManager::loadTexture(sf::Texture& texture, const std::string& path){
        if(!texture.loadFromFile(path)){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to load texture " + path);
        } else {
            Logger::println(Level::INFO, "Asset Manager", "Loaded texture " + path);
        }
    }

    void AssetManager::loadAudio(sf::SoundBuffer* audio, const std::string& path){
        if(!audio->loadFromFile(path)){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to load audio " + path);
        } else {
            Logger::println(Level::INFO, "Asset Manager", "Loaded audio " + path);
        }
    }

    void AssetManager::loadShader(sf::Shader* shader, const std::string& path){
        // Check if the main folder exists
        if(!std::filesystem::is_directory(path)){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to load shader " + path);
        } else {
            // Folder actually exists, load the optional vertex and fragment shaders
            Logger::print(Level::INFO, "Asset Manager", "Loading shader " + path);
            
            sf::FileInputStream vertexFileStream;
            sf::FileInputStream fragmentFileStream;
            bool vertexSuccess = vertexFileStream.open(path + "/vertex.glsl");
            bool fragmentSuccess = fragmentFileStream.open(path + "/fragment.glsl");

            if(vertexSuccess && fragmentSuccess){
                // Load both
                shader->loadFromStream(vertexFileStream, fragmentFileStream);

                Logger::printRaw("\tVertex...");
                Logger::printRaw("Loaded");
                Logger::printRaw("\tFragment...");
                Logger::printRaw("Loaded\n");
            } else if(vertexSuccess){
                // Load just vertex
                shader->loadFromStream(vertexFileStream, sf::Shader::Type::Vertex);

                Logger::printRaw("\tVertex...");
                Logger::printRaw("Loaded\n");
            } else if(fragmentSuccess){
                // Load just fragment
                shader->loadFromStream(fragmentFileStream, sf::Shader::Type::Fragment);

                Logger::printRaw("\tFragment...");
                Logger::printRaw("Loaded\n");
            } else {
                Logger::printRaw(" Not loaded\n");
            }
        }
    }

    void AssetManager::loadFont(sf::Font& font, const std::string& path){
        if(!font.loadFromFile(path)){
            Logger::println(Level::SEVERE, "Asset Manager", "Failed to load font " + path);
        } else {
            Logger::println(Level::INFO, "Asset Manager", "Loaded font " + path);
        }
    }
}