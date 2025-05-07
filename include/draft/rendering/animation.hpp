#pragma once

#include "draft/rendering/image.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/file_handle.hpp"
#include <vector>

namespace Draft {
    class Animation {
    private:
        // Variables
        Texture defaultTexture{Image()};
        Resource<Texture> texture{&defaultTexture};
        std::vector<FloatRect> frames;
        std::vector<float> frameTimes;
        float totalFrameTime = 0.f;

    public:
        // Constructors
        Animation(Assets& assets, const FileHandle& handle);
        Animation(const Animation& other) = delete;

        // Operators
        Animation& operator=(const Animation& other) = delete;

        // Functions
        TextureRegion get_frame(float frameTime = 0.f);
    };
};