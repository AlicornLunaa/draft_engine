#include "draft/rendering/pipeline/passes/overlay_pass.hpp"
#include "draft/rendering/pipeline/renderer.hpp"

namespace Draft {
    void OverlayPass::run(Renderer& renderer){
        renderer.begin_pass(*this);
        renderer.set_state(m_state);
        renderer.end_pass();
    }
}
