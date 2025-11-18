#pragma once

#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glad/gl.h"
#include <optional>

namespace Draft {
    struct RenderState {
        // Depth state
        bool depthTest = true;
        bool depthWrite = true;
        GLenum depthFunction = GL_LEQUAL;

        // Blending
        bool blend = false;
        GLenum blendSrc = GL_SRC_ALPHA;
        GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;
        GLenum blendEquation = GL_FUNC_ADD;

        // Rasterization
        bool cullFace = false;
        GLenum cullMode = GL_BACK;
        bool frontFaceCCW = true;
        bool polygonOffset = false;

        // Viewport and scissors
        std::optional<Rect<float>> viewport;
        std::optional<Rect<float>> scissor;

        // Drawing
        Vector4f clearColor = Vector4f(0, 0, 0, 1);
        GLbitfield clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    };
};