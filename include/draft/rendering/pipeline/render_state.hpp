#pragma once

#include "draft/math/glm.hpp"
#include "glad/gl.h"

namespace Draft {
    struct RenderState {
        // Depth state
        bool depthTest = true;
        bool depthWrite = true;
        GLenum depthFunction = GL_LESS;

        // Stenciling
        // bool stencilTest = false;
        // GLenum stencilFunction = GL_ALWAYS;
        // GLint stencilRef = 0;
        // GLuint stencilMask = 0xFF;

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

        // Viewport
        int viewportX = 0, viewportY = 0;
        int viewportWidth = 0, viewportHeight = 0;

        // Scissor
        bool scissorTest = false;
        int scissorX, scissorY, scissorWidth, scissorHeight;

        // Drawing
        Vector4f clearColor = Vector4f(0, 0, 0, 1);
        GLbitfield clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    };
};