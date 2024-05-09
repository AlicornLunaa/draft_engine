#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "draft/util/asset_manager.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    namespace Assets {
        // Structures
        template<typename T>
        struct Resource {
            std::unique_ptr<T> ptr;
            size_t package_owner_count = 0;

            template<typename ...Args>
            Resource(Args&&... args) : ptr(std::make_unique<T>(args...)) {}
        };

        struct AssetPackage {
        private:
            size_t assetCount = 0;
            std::vector<Resource<Texture>*> textures;
            std::vector<Resource<Image>*> images;
            std::vector<Resource<Model>*> models;
            std::vector<Resource<Shader>*> shaders;
            std::vector<Resource<Font>*> fonts;
            std::vector<Resource<SoundBuffer>*> audio;

        public:
            const size_t id;

            AssetPackage(size_t id) : id(id) {}
            AssetPackage(const AssetPackage& other) = delete;
            AssetPackage& operator=(const AssetPackage& other) = delete;
            ~AssetPackage(){
                for(auto res : textures) res->package_owner_count--;
                for(auto res : images) res->package_owner_count--;
                for(auto res : models) res->package_owner_count--;
                for(auto res : shaders) res->package_owner_count--;
                for(auto res : fonts) res->package_owner_count--;
                for(auto res : audio) res->package_owner_count--;
            }

            inline size_t get_asset_count() const { return assetCount; }

            void own(Resource<Texture>& res){
                // Adds this package as an owner for the resource
                textures.push_back(&res);
                assetCount++;
                res.package_owner_count++;
            }

            void own(Resource<Image>& res){
                // Adds this package as an owner for the resource
                images.push_back(&res);
                assetCount++;
                res.package_owner_count++;
            }

            void own(Resource<Model>& res){
                // Adds this package as an owner for the resource
                models.push_back(&res);
                assetCount++;
                res.package_owner_count++;
            }

            void own(Resource<Shader>& res){
                // Adds this package as an owner for the resource
                shaders.push_back(&res);
                assetCount++;
                res.package_owner_count++;
            }

            void own(Resource<Font>& res){
                // Adds this package as an owner for the resource
                fonts.push_back(&res);
                assetCount++;
                res.package_owner_count++;
            }

            void own(Resource<SoundBuffer>& res){
                // Adds this package as an owner for the resource
                audio.push_back(&res);
                assetCount++;
                res.package_owner_count++;
            }
        };

        // Variables
        std::unordered_map<size_t, AssetPackage> packages;
        std::unordered_map<std::string, Resource<Texture>> textures;
        std::unordered_map<std::string, Resource<Image>> images;
        std::unordered_map<std::string, Resource<Model>> models;
        std::unordered_map<std::string, Resource<Shader>> shaders;
        std::unordered_map<std::string, Resource<Font>> fonts;
        std::unordered_map<std::string, Resource<SoundBuffer>> audio;
        AssetPackage* currentPackage = nullptr;

        std::unique_ptr<Texture> MISSING_TEXTURE = nullptr;
        std::unique_ptr<Image> MISSING_IMAGE = nullptr;
        std::unique_ptr<Model> MISSING_MODEL = nullptr;
        std::unique_ptr<Shader> MISSING_SHADER = nullptr;
        std::unique_ptr<Font> MISSING_FONT = nullptr;
        std::unique_ptr<SoundBuffer> MISSING_AUDIO = nullptr;

        // Helper functions
        template<typename T>
        void remove_orphans(std::unordered_map<std::string, Resource<T>>& map){
            std::vector<std::string> deleteList;

            for(auto& [key, res] : map){
                if(res.package_owner_count <= 0)
                    // Mark for deletion
                    deleteList.push_back(key);
            }

            for(auto& key : deleteList){
                map.erase(key);
            }
        }

        // Function interface
        size_t start_package(){
            // Create a new package
            size_t id = packages.size();
            packages.emplace(id, 0);
            currentPackage = &packages.at(id);
            return id;
        }

        void select_package(size_t package){
            // Check if package exists
            assert(packages.find(package) != packages.end() && "Package selected does not exist");
            currentPackage = &packages.at(package);
        }

        void end_package(size_t package){
            auto iter = packages.find(package);
            assert(iter != packages.end() && "Package ended does not exist");

            // Remove the package
            packages.erase(iter);

            // Remove pointer if deleting this package
            if(package == currentPackage->id)
                currentPackage = nullptr;

            // Check each resource to see if orphaned resources are ready to be deleted
            remove_orphans(textures);
            remove_orphans(images);
            remove_orphans(models);
            remove_orphans(shaders);
            remove_orphans(fonts);
        }

        void end_package(){
            assert(currentPackage && "Cannot end a package with none selected");
            end_package(currentPackage->id);
        }

        template<>
        const Texture& get_missing_placeholder(){
            if(!MISSING_TEXTURE) MISSING_TEXTURE = std::make_unique<Texture>(FileHandle::automatic("assets/textures/missing_texture.png"));
            return *MISSING_TEXTURE;
        }

        template<>
        const Image& get_missing_placeholder(){
            if(!MISSING_IMAGE) MISSING_IMAGE = std::make_unique<Image>(FileHandle::automatic("assets/textures/missing_texture.png"));
            return *MISSING_IMAGE;
        }

        template<>
        const Model& get_missing_placeholder(){
            if(!MISSING_MODEL) MISSING_MODEL = std::make_unique<Model>(FileHandle::automatic("assets/models/missing_model.glb"));
            return *MISSING_MODEL;
        }

        template<>
        const Shader& get_missing_placeholder(){
            if(!MISSING_SHADER) MISSING_SHADER = std::make_unique<Shader>(FileHandle::automatic("assets/shaders/missing_shader/vertex.glsl"), FileHandle::automatic("assets/shaders/missing_shader/fragment.glsl"));
            return *MISSING_SHADER;
        }

        template<>
        const Font& get_missing_placeholder(){
            if(!MISSING_FONT) MISSING_FONT = std::make_unique<Font>(FileHandle::automatic("assets/fonts/missing_font.ttf"));
            return *MISSING_FONT;
        }

        template<>
        const SoundBuffer& get_missing_placeholder(){
            if(!MISSING_AUDIO) MISSING_AUDIO = std::make_unique<SoundBuffer>(FileHandle::automatic("assets/audio/missing_audio.wav"));
            return *MISSING_AUDIO;
        }

        template<>
        const Texture& get_asset(const FileHandle& handle){
            // If no package currently exists, start one
            if(!currentPackage)
                start_package();

            // Load or retrieve an external texture
            std::string str = handle.get_path();

            if(textures.find(str) == textures.end()){
                // No texture exists by this name, try loading it
                textures.emplace(str, Resource<Texture>(handle));
                
                if(!textures.at(str).ptr->is_loaded()){
                    // Texture failed to load, give it the missing texture
                    return get_missing_placeholder<Texture>();
                }
                
                currentPackage->own(textures.at(str));
            }

            return *textures.at(str).ptr;
        }

        template<>
        const Image& get_asset(const FileHandle& handle){
            // If no package currently exists, start one
            if(!currentPackage)
                start_package();

            // Load or retrieve an external texture
            std::string str = handle.get_path();

            if(images.find(str) == images.end()){
                // No image exists by this name, try loading it
                try {
                    images.emplace(str, Resource<Image>(handle));
                } catch(int e){
                    Logger::print(Level::SEVERE, "Image", std::to_string(e));
                    return get_missing_placeholder<Image>();
                }

                currentPackage->own(images.at(str));
            }

            return *images.at(str).ptr;
        }

        template<>
        const Model& get_asset(const FileHandle& handle){
            // If no package currently exists, start one
            if(!currentPackage)
                start_package();

            // Load or retrieve an external model
            std::string str = handle.get_path();

            if(models.find(str) == models.end()){
                // No model exists by this name, try loading it
                try {
                    models.emplace(str, Resource<Model>(handle));
                } catch(int e){
                    Logger::print(Level::SEVERE, "Model", std::to_string(e));
                    return get_missing_placeholder<Model>();
                }

                currentPackage->own(models.at(str));
            }

            return *models.at(str).ptr;
        }

        template<>
        const Shader& get_asset(const FileHandle& handle){
            // If no package currently exists, start one
            if(!currentPackage)
                start_package();

            // Load or retrieve an external shader
            std::string str = handle.get_path();

            if(shaders.find(str) == shaders.end()){
                // No shader exists by this name, try loading it
                try {
                    shaders.emplace(str, Resource<Shader>(handle));
                } catch(int e){
                    Logger::print(Level::SEVERE, "Shader", std::to_string(e));
                    return get_missing_placeholder<Shader>();
                }

                currentPackage->own(shaders.at(str));
            }

            return *shaders.at(str).ptr;
        }

        template<>
        const Font& get_asset(const FileHandle& handle){
            // If no package currently exists, start one
            if(!currentPackage)
                start_package();

            // Load or retrieve an external font
            std::string str = handle.get_path();

            if(fonts.find(str) == fonts.end()){
                // No font exists by this name, try loading it
                try {
                    fonts.emplace(str, Resource<Font>(handle));
                } catch(int e){
                    Logger::print(Level::SEVERE, "Font", std::to_string(e));
                    return get_missing_placeholder<Font>();
                }

                currentPackage->own(fonts.at(str));
            }

            return *fonts.at(str).ptr;
        }

        template<>
        const SoundBuffer& get_asset(const FileHandle& handle){
            // If no package currently exists, start one
            if(!currentPackage)
                start_package();

            // Load or retrieve an external font
            std::string str = handle.get_path();

            if(audio.find(str) == audio.end()){
                // No sound exists by this name, try loading it
                try {
                    audio.emplace(str, Resource<SoundBuffer>(handle));
                } catch(int e){
                    Logger::print(Level::SEVERE, "Audio", std::to_string(e));
                    return get_missing_placeholder<SoundBuffer>();
                }

                currentPackage->own(audio.at(str));
            }

            return *audio.at(str).ptr;
        }

        void reload(){
            Logger::print(Level::INFO, "Asset Manager", "Reloading...");
            for(auto& res : textures) res.second.ptr->reload();
            for(auto& res : models) res.second.ptr->reload();
            for(auto& res : shaders) res.second.ptr->reload();
            for(auto& res : fonts) res.second.ptr->reload();
            for(auto& res : audio) res.second.ptr->reload();
            Logger::print_raw("Complete\n");
        }

        void cleanup(){
            // Cleans all resources
            packages.clear();
            currentPackage = nullptr;
            remove_orphans(textures);
            remove_orphans(images);
            remove_orphans(models);
            remove_orphans(shaders);
            remove_orphans(fonts);
            remove_orphans(audio);
        }
    }
}