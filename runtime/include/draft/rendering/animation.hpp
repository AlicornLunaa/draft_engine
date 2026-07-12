#pragma once

#include "draft/asset/resource.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/files/file_handle.hpp"

#include <vector>

namespace Draft {
    /**
     * @brief A sprite-sheet animation, a fixed texture plus a list of frame regions/durations
     * parsed from an Aseprite/TexturePacker-shaped JSON manifest
     */
    class Animation {
    private:
        // Variables
        Resource<Texture> texture;
        std::vector<FloatRect> frames;
        std::vector<float> frameTimes;
        float totalFrameTime = 0.f;

    public:
        // Constructors
        Animation(Resource<Texture> texture, const FileHandle& handle);
        Animation(const Animation& other) = delete;

        // Operators
        Animation& operator=(const Animation& other) = delete;

        // Functions
        /**
         * @throws std::runtime_error if this animation has no frames.
         */
        TextureRegion get_frame(float frameTime = 0.f) const;
    };
}
