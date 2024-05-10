#pragma once

#include "draft/rendering/texture.hpp"
#include "draft/util/file_handle.hpp"
#include <vector>

namespace Draft {
    class Animation {
    private:
        // Variables
        Texture const* texture = nullptr;
        std::vector<FloatRect> frames;
        std::vector<float> frameTimes;
        float totalFrameTime = 0.f;

    public:
        // Constructors
        Animation(const FileHandle& handle);
        Animation(const Animation& other) = delete;

        // Operators
        Animation& operator=(const Animation& other) = delete;

        // Functions
        const TextureRegion get_frame(float frameTime = 0.f) const;
    };
};