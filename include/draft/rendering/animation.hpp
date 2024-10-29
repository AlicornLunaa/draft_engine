#pragma once

#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/file_handle.hpp"
#include <vector>

namespace Draft {
    class Animation {
    private:
        // Variables
        Resource<Texture> texture = Assets::manager.get<Texture>("assets/textures/debug_white.png", true);
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
        TextureRegion get_frame(float frameTime = 0.f);
    };
};