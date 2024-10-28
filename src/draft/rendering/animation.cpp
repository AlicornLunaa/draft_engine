#include "draft/rendering/animation.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "nlohmann/json.hpp"

#include <string>

using json = nlohmann::json;

namespace Draft {
    // Constructor
    Animation::Animation(const FileHandle& handle){
        // Parse the JSON
        json data = json::parse(handle.read_string());

        // Get the texture location & data
        std::string texturePath = "assets/textures/" + static_cast<std::string>(data["meta"]["image"]);
        texture = Assets::manager.get<Texture>(texturePath, true);

        // Create each frame region
        for(json frameData : data["frames"]){
            json bounds = frameData["frame"];
            float duration = frameData["duration"];

            totalFrameTime += duration;
            frameTimes.push_back(totalFrameTime);

            frames.push_back({
                bounds["x"],
                texture->get_size().y - static_cast<float>(bounds["y"]) - static_cast<float>(bounds["h"]),
                bounds["w"],
                bounds["h"]
            });
        }
    }

    // Functions
    const TextureRegion Animation::get_frame(float frameTime) const {
        // Keep time
        frameTime = fmodf(frameTime, totalFrameTime);

        // Get required frame
        for(size_t i = 0; i < frames.size(); i++){
            if(frameTimes[i] > frameTime){
                return { texture, frames[i] };
            }
        }

        // Error
        return { texture, frames[(int)(frameTime / 100) % 20] };
    }
};