#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/util/time.hpp"

namespace Draft {
    namespace UI {
        struct Style {
            // Stylesheet
            Vector4f backgroundColor = { 1, 1, 1, 1 };
            Vector4f textColor = { 0, 0, 0, 1 };
        };

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
        };

        enum class XAnchor { LEFT, CENTER, RIGHT };
        enum class YAnchor { TOP, MIDDLE, BOTTOM };
    };
};