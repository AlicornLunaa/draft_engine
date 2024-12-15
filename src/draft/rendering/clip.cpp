#include "draft/rendering/clip.hpp"
#include "draft/aliasing/filter.hpp"

namespace Draft {
    // Static data
    Clip* Clip::currentClip = nullptr;

    // Private functions
    void Clip::cut(){
        // Sets the scissor to the box, this is a seperate function so it can be called without changing GL_SCISSOR_TEST
        glScissor(box.x, box.y, box.width, box.height);
    }

    // Functions
    void Clip::begin(){
        // Start the clip
        if(!currentClip)
            glEnable(GL_SCISSOR_TEST);
    
        // Save state so hierarchical clipping works
        previousClip = currentClip;
        currentClip = this;

        // Set clip parameters
        cut();
    }

    void Clip::end(){
        // This clip ends, send it up to the previous clip
        if(previousClip) previousClip->cut();
        currentClip = previousClip;
        previousClip = nullptr;

        // Cleanup by disabling GL_SCISSOR_TEST if no more clips
        if(!currentClip)
            glDisable(GL_SCISSOR_TEST);
    }
};