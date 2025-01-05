#pragma once

#include "draft/core/application.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"

namespace Draft {
    namespace UI {
        struct Context {
            Application* app;
            SpriteBatch& batch;
            TextRenderer& textBatch;
            FloatRect bounds; // The inner container size
        };
    };
};