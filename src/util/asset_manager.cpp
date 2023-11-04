#include <fstream>
#include <filesystem>

#include "asset_manager.hpp"
#include "ansi_colors.hpp"
#include "logger.hpp"

using namespace SpaceGame;
using namespace Util;

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
}

void AssetManager::reload(){
    Logger::println(Logger::Level::INFO, "Asset Manager", "Reloading...");

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

sf::Texture& AssetManager::getTexture(const std::string& name) const {
    const auto& result = textureMap.find(name);

    if(result == textureMap.end()){
        Logger::println(Logger::Level::SEVERE, "Asset Manager", "Failed to find texture " + name);
        return const_cast<sf::Texture&>(textureArray[0]);
    }
    
    // I know, evil scary const_cast but I really didnt wanna fight with the array
    return const_cast<sf::Texture&>(textureArray[textureMap.at(name)]);
}

const sf::SoundBuffer& AssetManager::getAudio(const std::string& name) const {
    const auto& result = audioMap.find(name);

    if(result == audioMap.end()){
        Logger::println(Logger::Level::SEVERE, "Asset Manager", "Failed to find audio " + name);
        return *audioArray[0];
    }
    
    return *audioArray[audioMap.at(name)];
}

sf::Shader& AssetManager::getShader(const std::string& name) const {
    const auto& result = shaderMap.find(name);

    if(result == shaderMap.end()){
        Logger::println(Logger::Level::SEVERE, "Asset Manager", "Failed to find shader " + name);
        return *shaderArray[0];
    }
    
    return *shaderArray[shaderMap.at(name)];
}

// Private functionsz
void AssetManager::loadTexture(sf::Texture& texture, const std::string& path){
    if(!texture.loadFromFile(path)){
        Logger::println(Logger::Level::SEVERE, "Asset Manager", "Failed to load texture " + path);
    } else {
        Logger::println(Logger::Level::INFO, "Asset Manager", "Loaded texture " + path);
    }
}

void AssetManager::loadAudio(sf::SoundBuffer* audio, const std::string& path){
    if(!audio->loadFromFile(path)){
        Logger::println(Logger::Level::SEVERE, "Asset Manager", "Failed to load audio " + path);
    } else {
        Logger::println(Logger::Level::INFO, "Asset Manager", "Loaded audio " + path);
    }
}

void AssetManager::loadShader(sf::Shader* shader, const std::string& path){
    // Check if the main folder exists
    if(!std::filesystem::is_directory(path)){
        Logger::println(Logger::Level::SEVERE, "Asset Manager", "Failed to load shader " + path);
    } else {
        // Folder actually exists, load the optional vertex and fragment shaders
        Logger::println(Logger::Level::INFO, "Asset Manager", "Loading shader " + path);
        sf::FileInputStream fileStream;

        // Vertex loading
        Logger::printRaw(std::string(Color::Reset) + "\tVertex...");
        if(fileStream.open(path + "/vertex.glsl")){
            // Vertex shader exists, read it into memory and load the shader
            shader->loadFromStream(fileStream, sf::Shader::Type::Vertex);
            Logger::printRaw(std::string(Color::Green) + "Loaded");
        } else {
            Logger::printRaw(std::string(Color::Red) + "Not found");
        }
        Logger::printRaw("\n");

        // Fragment loading
        Logger::printRaw(std::string(Color::Reset) + "\tFragment...");
        if(fileStream.open(path + "/fragment.glsl")){
            // Vertex shader exists, read it into memory and load the shader
            shader->loadFromStream(fileStream, sf::Shader::Type::Fragment);
            Logger::printRaw(std::string(Color::Green) + "Loaded");
        } else {
            Logger::printRaw(std::string(Color::Red) + "Not found");
        }
        Logger::printRaw(std::string(Color::Reset) + "\n");
    }
}
