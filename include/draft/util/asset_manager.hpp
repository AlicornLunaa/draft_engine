#pragma once

#include "draft/rendering/model.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"

#include <queue>
#include <unordered_map>
#include <vector>

namespace Draft {
    // Owns and manages all resources inside the game
    class AssetManager {
    private:
        // Static variables
        static Texture* MISSING_TEXTURE;
        static Texture* EMPTY_NORMAL_MAP;
        static Texture* DEBUG_WHITE;
        static Texture* DEBUG_BLACK;
        static Model* MISSING_MODEL;
        static Shader* MISSING_SHADER;

        // Variables
        std::queue<FileHandle> textureQueue;
        std::vector<Texture*> textureArray;
        std::unordered_map<std::string, size_t> textureMap;

        std::queue<FileHandle> modelQueue;
        std::vector<Model*> modelArray;
        std::unordered_map<std::string, size_t> modelMap;

        std::queue<FileHandle> shaderQueue;
        std::vector<Shader*> shaderArray;
        std::unordered_map<std::string, size_t> shaderMap;

        // Private functions
        static Texture* load_static_texture(const std::string& path);
        static Model* load_static_model(const std::string& path);
        static Shader* load_static_shader(const std::string& vertex, const std::string& fragment);

    public:
        AssetManager();
        AssetManager(const AssetManager& other) = delete;
        ~AssetManager();

        void clear();
        void reload();
        void load();

        inline void queue_texture(const FileHandle& path){ textureQueue.emplace(path); }
        inline void queue_model(const FileHandle& path){ modelQueue.emplace(path); }
        inline void queue_shader(const FileHandle& path){ shaderQueue.emplace(path); }

        const Texture& get_texture(const std::string& name) const;
        const Model& get_model(const std::string& name) const;
        Shader& get_shader(const std::string& name) const;

        // Static variables
        static const Texture& get_missing_texture();
        static const Texture& get_empty_normal_map();
        static const Texture& get_debug_white();
        static const Texture& get_debug_black();
        static const Model& get_missing_model();
        static Shader& get_missing_shader();
    };
}