#pragma once

#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace Draft {
    // Owns and manages all resources inside the game
    class AssetManager {
    private:
        // Static variables
        static std::unique_ptr<Texture> MISSING_TEXTURE;
        static std::unique_ptr<Texture> EMPTY_NORMAL_MAP;
        static std::unique_ptr<Texture> DEBUG_WHITE;
        static std::unique_ptr<Texture> DEBUG_BLACK;

        // Variables
        std::vector<std::string> textureQueue;
        std::vector<Texture*> textureArray;
        std::unordered_map<std::string, size_t> textureMap;

        std::vector<std::string> shaderQueue;
        std::vector<Shader*> shaderArray;
        std::unordered_map<std::string, size_t> shaderMap;

        // std::vector<std::string> audioQueue;
        // std::vector<sf::SoundBuffer*> audioArray;
        // std::unordered_map<std::string, size_t> audioMap;

        // std::vector<std::string> fontQueue;
        // std::vector<sf::Font> fontArray;
        // std::unordered_map<std::string, size_t> fontMap;

    public:
        AssetManager();
        ~AssetManager();

        void clear();
        void reload();
        void load();

        void queue_texture(const std::string& path);
        void queue_shader(const std::string& path);
        // void queueAudio(const std::string& path);
        // void queueFont(const std::string& path);

        const Texture& get_texture(const std::string& name) const;

        Shader& get_shader(const std::string& name) const;
        // const sf::SoundBuffer& getAudio(const std::string& name) const;
        // const sf::Font& getFont(const std::string& name) const;

        // Static variables
        static const Texture& get_missing_texture();
        static const Texture& get_empty_normal_map();
        static const Texture& get_debug_white();
        static const Texture& get_debug_black();

    private:
        static std::unique_ptr<Texture> load_static_texture(const std::string& path);
        void load_texture(Texture* texture, const std::string& path);
        void load_shader(Shader* shader, const std::string& path);
        // void loadAudio(sf::SoundBuffer* audio, const std::string& path);
        // void loadFont(sf::Font& shader, const std::string& path);
    };
}