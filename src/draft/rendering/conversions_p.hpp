#pragma once

#include "draft/rendering/texture.hpp"
#include "draft/rendering/image.hpp"

namespace Draft {
    int wrap_to_gl(Wrap wrap);
    int filter_to_gl(Filter filter);
    int color_space_to_gl(ColorSpace cs);
    ColorSpace channels_to_color_space(int channels);
};