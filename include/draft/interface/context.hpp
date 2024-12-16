#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/interface/style.hpp"
#include "draft/util/time.hpp"

namespace Draft {
    namespace UI {
        struct Context {
            // Holds information to convert interface-space coordinates to screen-space for scissor clipping.
            // Also contains information for the 'canvas size' so that it can be rendered on the either the
            // master interface or the parent respectively.
            const Matrix4& projection;
            const Vector2u windowSize;
            const Time& dt;
            SpriteBatch& batch;
            TextRenderer& text;
            Vector2f parentSize; // Used for relative positioning
            Stylesheet& stylesheet;
            std::string styleStack = "";
        };
    };
};