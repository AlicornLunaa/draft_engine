#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/util/time.hpp"

namespace Draft {
    namespace UI {
        enum Anchor { TOP, MIDDLE, BOTTOM, LEFT, CENTER, RIGHT };

        struct Style {
            // Common stylesheet for a ui interface
            Anchor horizontalAnchor = LEFT;
            Anchor verticalAnchor = TOP;
            
            Vector4f textColor = {1, 1, 1, 1};
            Font const* font = nullptr;

            Vector4f activeColor = {0.4, 0.8, 0.4, 1};
            Vector4f inactiveColor = {0.8, 0.4, 0.4, 1};
            Vector4f disabledColor = {0.4, 0.4, 0.4, 1};

            Vector2f padding{0.f};
            Vector4f margin{0.f};
            
            Vector4f backgroundColor = {0.2, 0.2, 0.2, 1};
            Texture const* background = nullptr;
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
            Style style;
        };
    };
};