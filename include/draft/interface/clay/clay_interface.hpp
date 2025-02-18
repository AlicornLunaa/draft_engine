#pragma once

#include "draft/core/application.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/clip.hpp"
#include "draft/rendering/font.hpp"

#include "clay.h"
#include "draft/rendering/shader.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    class Clay {
    private:
        // Variables
        Resource<Shader> uiShader = Assets::manager.get<Shader>("assets/shaders/default");
        std::vector<Resource<Font>> fonts;
        TextRenderer textBatch;
        OrthoCamera camera;
        Application& appRef;
        Clip scissor;

        uint64_t totalMemorySize;
        Clay_Arena arena;

        // Private functions
        static Clay_Dimensions measure_text(Clay_String* text, Clay_TextElementConfig* config);

    public:
        // Constructors
        Clay(Application& app);
        ~Clay();
        
        // Functions
        uint load_font(const Resource<Font>& font);
        void begin(SpriteBatch& batch, const Time& deltaTime);
        void end();
    };
};