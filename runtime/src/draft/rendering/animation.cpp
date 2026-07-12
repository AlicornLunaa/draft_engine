#include "draft/rendering/animation.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/json.hpp"

#include <cmath>
#include <stdexcept>

namespace Draft {
    // Constructor
    Animation::Animation(Resource<Texture> texture, const FileHandle& handle) : texture(texture) {
        // Parse the JSON
        JSON data(handle);

        // Create each frame region. The texture itself is supplied by the caller
        for(JSON frameData : data["frames"]){
            JSON bounds = frameData["frame"];
            float duration = frameData["duration"];

            totalFrameTime += duration;
            frameTimes.push_back(totalFrameTime);

            frames.push_back({
                bounds["x"],
                this->texture->get_properties().size.y - static_cast<float>(bounds["y"]) - static_cast<float>(bounds["h"]),
                bounds["w"],
                bounds["h"]
            });
        }
    }

    // Functions
    TextureRegion Animation::get_frame(float frameTime) const {
        if(frames.empty())
            throw std::runtime_error("Animation::get_frame(): animation has no frames");

        // Keep time
        if(totalFrameTime > 0.f)
            frameTime = fmodf(frameTime, totalFrameTime);

        // Get required frame
        for(size_t i = 0; i < frames.size(); i++){
            if(frameTimes[i] > frameTime){
                return { texture, frames[i] };
            }
        }

        // Defensive fallback for floating-point rounding leaving frameTime a hair past the last frame's own end time
        return { texture, frames.back() };
    }
}
