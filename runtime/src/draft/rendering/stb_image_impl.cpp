// Dedicated translation unit for stb_image/stb_image_write's implementation. The old engine
// defined STB_IMAGE_IMPLEMENTATION/STB_IMAGE_WRITE_IMPLEMENTATION in texture.cpp/model.cpp -
// files unrelated to Image itself - so Image's own link-ability implicitly depended on whichever
// of those happened to also be in the build. Giving it one dedicated file removes that.
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
