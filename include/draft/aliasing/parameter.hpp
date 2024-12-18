#pragma once

#include "glad/gl.h"

namespace Draft {
    enum Parameter { 
        DEPTH_STENCIL_TEXTURE_MODE = GL_DEPTH_STENCIL_TEXTURE_MODE,
        TEXTURE_BASE_LEVEL = GL_TEXTURE_BASE_LEVEL,
        TEXTURE_COMPARE_FUNC = GL_TEXTURE_COMPARE_FUNC,
        TEXTURE_COMPARE_MODE = GL_TEXTURE_COMPARE_MODE,
        TEXTURE_LOD_BIAS = GL_TEXTURE_LOD_BIAS,
        TEXTURE_MIN_FILTER = GL_TEXTURE_MIN_FILTER,
        TEXTURE_MAG_FILTER = GL_TEXTURE_MAG_FILTER,
        TEXTURE_MIN_LOD = GL_TEXTURE_MIN_LOD,
        TEXTURE_MAX_LOD = GL_TEXTURE_MAX_LOD,
        TEXTURE_MAX_LEVEL = GL_TEXTURE_MAX_LEVEL,
        TEXTURE_SWIZZLE_R = GL_TEXTURE_SWIZZLE_R,
        TEXTURE_SWIZZLE_G = GL_TEXTURE_SWIZZLE_G,
        TEXTURE_SWIZZLE_B = GL_TEXTURE_SWIZZLE_B,
        TEXTURE_SWIZZLE_A = GL_TEXTURE_SWIZZLE_A,
        TEXTURE_WRAP_S = GL_TEXTURE_WRAP_S,
        TEXTURE_WRAP_T = GL_TEXTURE_WRAP_T,
        TEXTURE_WRAP_R = GL_TEXTURE_WRAP_R
    };
};