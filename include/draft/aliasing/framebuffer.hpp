#pragma once

#include "glad/gl.h"

namespace Draft {
    enum FramebufferTarget {
        FRAMEBUFFER = GL_FRAMEBUFFER,
        DRAW = GL_DRAW_FRAMEBUFFER,
        READ = GL_READ_FRAMEBUFFER
    };
};