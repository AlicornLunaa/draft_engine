#include "draft/util/asset_loaders.hpp"
#include "draft/rendering/particle_system.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager.hpp"
#include "draft/util/file_handle.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Draft {
    // Particle loader
    std::shared_ptr<void> ParticleLoader::load_sync() const {
        // Default to basic call of default filehandle constructor
        try {
            auto ptr = std::shared_ptr<ParticleProps>(new ParticleProps(), [](void* ptr){ delete static_cast<ParticleProps*>(ptr); });
            json json = json::parse(handle.read_string());

            ptr->velocity.x = json["velocity"]["x"];
            ptr->velocity.y = json["velocity"]["y"];
            ptr->velocityVariation.x = json["velocity_variation"]["x"];
            ptr->velocityVariation.y = json["velocity_variation"]["y"];
            ptr->colorBegin.r = json["color_begin"]["r"];
            ptr->colorBegin.g = json["color_begin"]["g"];
            ptr->colorBegin.b = json["color_begin"]["b"];
            ptr->colorBegin.a = json["color_begin"]["a"];
            ptr->colorEnd.r = json["color_end"]["r"];
            ptr->colorEnd.g = json["color_end"]["g"];
            ptr->colorEnd.b = json["color_end"]["b"];
            ptr->colorEnd.a = json["color_end"]["a"];
            ptr->sizeBegin = json["size_begin"];
            ptr->sizeEnd = json["size_end"];
            ptr->sizeVariation = json["size_variation"];
            ptr->lifeTime = json["lifetime"];
            ptr->texture = Assets::manager.get<Texture>(json["texture"], true);

            return ptr;
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(ParticleLoader).name(), std::to_string(e));
        }

        return nullptr;
    }

    void ParticleLoader::load_async(){
        auto ptr = new ParticleProps();
        json data = json::parse(handle.read_string());

        ptr->velocity.x = data["velocity"]["x"];
        ptr->velocity.y = data["velocity"]["y"];
        ptr->velocityVariation.x = data["velocity_variation"]["x"];
        ptr->velocityVariation.y = data["velocity_variation"]["y"];
        ptr->colorBegin.r = data["color_begin"]["r"];
        ptr->colorBegin.g = data["color_begin"]["g"];
        ptr->colorBegin.b = data["color_begin"]["b"];
        ptr->colorBegin.a = data["color_begin"]["a"];
        ptr->colorEnd.r = data["color_end"]["r"];
        ptr->colorEnd.g = data["color_end"]["g"];
        ptr->colorEnd.b = data["color_end"]["b"];
        ptr->colorEnd.a = data["color_end"]["a"];
        ptr->sizeBegin = data["size_begin"];
        ptr->sizeEnd = data["size_end"];
        ptr->sizeVariation = data["size_variation"];
        ptr->lifeTime = data["lifetime"];

        texture = data["texture"];
        propPtr = ptr;
    }

    std::shared_ptr<void> ParticleLoader::finish_async_gl(){
        auto ptr = std::shared_ptr<ParticleProps>(propPtr, [](void* ptr){ delete static_cast<ParticleProps*>(ptr); });;
        ptr->texture = Assets::manager.get<Texture>(texture, true);
        return ptr;
    }

    Assets::BaseLoader* ParticleLoader::clone(const FileHandle& handle) const {
        auto* ptr = new ParticleLoader();
        ptr->handle = handle;
        return ptr;
    }

    // JSON Loader
    std::shared_ptr<void> JSONLoader::load_sync() const {
        // Default to basic call of default filehandle constructor
        try {
            auto ptr = std::shared_ptr<nlohmann::json>(new nlohmann::json(), [](void* ptr){ delete static_cast<nlohmann::json*>(ptr); });
            *ptr = json::parse(handle.read_string());
            return ptr;
        } catch(int e){
            Logger::print(Level::SEVERE, typeid(JSONLoader).name(), std::to_string(e));
        }

        return nullptr;
    }

    void JSONLoader::load_async(){
        data = new nlohmann::json();
        *data = json::parse(handle.read_string());
    }

    std::shared_ptr<void> JSONLoader::finish_async_gl(){
        auto ptr = std::shared_ptr<nlohmann::json>(data, [](void* ptr){ delete static_cast<nlohmann::json*>(ptr); });
        return ptr;
    }

    Assets::BaseLoader* JSONLoader::clone(const FileHandle& handle) const {
        auto* ptr = new JSONLoader();
        ptr->handle = handle;
        return ptr;
    }
};