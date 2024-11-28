#include "draft/aliasing/format.hpp"

namespace Draft {
    ColorFormat channels_to_color_format(int channels){
        switch(channels){
        case 1:
            return GREYSCALE;

        case 2:
            return RG;

        case 3:
            return RGB;
        
        case 4:
            return RGBA;

        default:
        case 0:
            assert(false && "Channels cant be deduced from 0");
            break;
        }
    }

    size_t color_format_to_bytes(ColorFormat format){
        switch(format){
        case DEPTH_COMPONENT:
            return 1;

        case DEPTH_COMPONENT16:
            return 2;

        case DEPTH_COMPONENT24:
            return 3;
        
        case DEPTH_STENCIL:
            return 1;
        
        case GREYSCALE:
            return 1;
        
        case RG:
            return 2;
        
        case RGB:
            return 3;
        
        case RGBA:
            return 4;

        default:
            assert(false && "Stride cant be deduced");
            break;
        }
    }
};