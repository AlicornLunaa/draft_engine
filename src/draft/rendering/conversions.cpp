#include "draft/rendering/conversions_p.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/texture.hpp"
#include "glad/gl.h"

namespace Draft {
    int wrap_to_gl(Wrap wrap){
        switch(wrap){
        default:
        case REPEAT:
            return GL_REPEAT;

        case MIRRORED_REPEAT:
            return GL_MIRRORED_REPEAT;

        case CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;

        case CLAMP_TO_BORDER:
            return GL_CLAMP_TO_BORDER;
        }
    }

    int filter_to_gl(Filter filter){
        switch(filter){
        default:
        case NEAREST:
            return GL_NEAREST;

        case LINEAR:
            return GL_LINEAR;
        }
    }

    int color_space_to_gl(ColorSpace cs){
        switch(cs){
        default:
        case ColorSpace::GREYSCALE:
            return GL_RED;

        case ColorSpace::RGB:
            return GL_RGB;

        case ColorSpace::RGBA:
            return GL_RGBA;
        }
    }

    ColorSpace channels_to_color_space(int channels){
        if(channels == 1) return ColorSpace::GREYSCALE;
        else if(channels == 3) return ColorSpace::RGB;
        else return ColorSpace::RGBA;
    }
};