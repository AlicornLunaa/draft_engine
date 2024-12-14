#include "draft/rendering/clip.hpp"
#include "draft/aliasing/filter.hpp"

namespace Draft {
    // Functions
    void Clip::begin(){
        // Start the clip
        glEnable(GL_SCISSOR_TEST);
        glScissor(box.x, box.y, box.width, box.height);
    }

    void Clip::end(){
        glDisable(GL_SCISSOR_TEST);
    }
};