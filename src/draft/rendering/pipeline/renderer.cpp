#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/pipeline/render_state.hpp"
#include <cassert>

namespace Draft {
    /// Abstract class
    // Constructor
    Renderer::Renderer(){
        // Set default state to start
        set_state(RenderState{}, true);
    }

    // Functions
    void Renderer::begin_pass(RenderPass& pass){
        // Initialize this pass by setting the state
        assert(!m_currentPass && "Previous pass must be ended before starting another");
        set_state(pass.get_render_state());
        m_currentPass = &pass;
    }

    void Renderer::end_pass(){
        assert(m_currentPass && "Cannot end a pass that has not started");
        m_currentPass = nullptr;
    }

    void Renderer::set_state(const RenderState& newState, bool force){
        // Depth
        if(force || newState.depthTest != m_previousState.depthTest) (newState.depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
        if(force || newState.depthWrite != m_previousState.depthWrite) glDepthMask(newState.depthWrite ? GL_TRUE : GL_FALSE);
        if(force || newState.depthFunction != m_previousState.depthFunction) glDepthFunc(newState.depthFunction);

        // Blending
        if(force || newState.blend != m_previousState.blend) (newState.blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND));
        if(force || newState.blend || m_previousState.blend){
            if(force || newState.blendSrc != m_previousState.blendSrc || newState.blendDst != m_previousState.blendDst){
                glBlendFunc(newState.blendSrc, newState.blendDst);
            }

            if(force || newState.blendEquation != m_previousState.blendEquation){
                glBlendEquation(newState.blendEquation);
            }
        }

        // Culling
        if(force || newState.cullFace != m_previousState.cullFace) (newState.cullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));
        if(force || newState.cullFace && newState.cullMode != m_previousState.cullMode) glCullFace(newState.cullMode);
        if(force || newState.frontFaceCCW != m_previousState.frontFaceCCW) glFrontFace(newState.frontFaceCCW ? GL_CCW : GL_CW);

        // Polygon offset
        if(force || newState.polygonOffset != m_previousState.polygonOffset) (newState.polygonOffset ? glEnable(GL_POLYGON_OFFSET_FILL) : glDisable(GL_POLYGON_OFFSET_FILL));

        // Viewport
        if(force || newState.viewportWidth != m_previousState.viewportWidth ||
                newState.viewportHeight != m_previousState.viewportHeight ||
                newState.viewportX != m_previousState.viewportX ||
                newState.viewportY != m_previousState.viewportY){
            glViewport(newState.viewportX, newState.viewportY, newState.viewportWidth, newState.viewportHeight);
        }

        // Scissor
        if(force || newState.scissorTest != m_previousState.scissorTest) (newState.scissorTest ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST));
        if(force || newState.scissorTest && (newState.scissorX != m_previousState.scissorX || newState.scissorY != m_previousState.scissorY || newState.scissorWidth != m_previousState.scissorWidth || newState.scissorHeight != m_previousState.scissorHeight)){
            glScissor(newState.scissorX, newState.scissorY, newState.scissorWidth, newState.scissorHeight);
        }

        // Clear color
        if(force || newState.clearColor != m_previousState.clearColor){
            glClearColor(newState.clearColor.x, newState.clearColor.y, newState.clearColor.z, newState.clearColor.w);
        }

        // Make copy for keeping state tracked
        m_previousState = newState;
    }

    /// Generic implementation
    void DefaultRenderer::render_frame(Scene& scene){
        m_geometryPass.run(*this, scene);
    }
};