#pragma once

#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace Draft {
    // Owns and manages all resources inside the game
    class AssetManager {
    private:
        std::vector<std::string> textureQueue;
        std::vector<sf::Texture> textureArray;
        std::unordered_map<std::string, size_t> textureMap;

        std::vector<std::string> audioQueue;
        std::vector<sf::SoundBuffer*> audioArray;
        std::unordered_map<std::string, size_t> audioMap;

        std::vector<std::string> shaderQueue;
        std::vector<sf::Shader*> shaderArray;
        std::unordered_map<std::string, size_t> shaderMap;

        std::vector<std::string> fontQueue;
        std::vector<sf::Font> fontArray;
        std::unordered_map<std::string, size_t> fontMap;

    public:
        AssetManager();
        ~AssetManager();

        void clear();
        void reload();
        void load();

        void queueTexture(const std::string& path);
        void queueAudio(const std::string& path);
        void queueShader(const std::string& path);
        void queueFont(const std::string& path);

        sf::Texture& getTexture(const std::string& name) const;
        const sf::SoundBuffer& getAudio(const std::string& name) const;
        sf::Shader& getShader(const std::string& name) const;
        const sf::Font& getFont(const std::string& name) const;

    private:
        void loadTexture(sf::Texture& texture, const std::string& path);
        void loadAudio(sf::SoundBuffer* audio, const std::string& path);
        void loadShader(sf::Shader* shader, const std::string& path);
        void loadFont(sf::Font& shader, const std::string& path);
    };
}