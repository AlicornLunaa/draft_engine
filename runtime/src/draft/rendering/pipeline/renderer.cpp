#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/vertex_array.hpp"
#include "draft/util/files/asset_file_system.hpp"

#include <array>
#include <cassert>

namespace Draft {
    // Helper data
    constexpr std::array<Draft::Vector2f, 8> QUAD_VERTICES = {
        Draft::Vector2f(-1, -1), Draft::Vector2f(0, 0), // Bottom-left
        Draft::Vector2f(1, -1), Draft::Vector2f(1, 0), // Bottom-right
        Draft::Vector2f(1, 1), Draft::Vector2f(1, 1), // Top-right
        Draft::Vector2f(-1, 1), Draft::Vector2f(0, 1) // Top-left
    };
    constexpr std::array<int, 6> QUAD_INDICES = { 0, 1, 2, 2, 3, 0 };

    /// Abstract class
    // Constructor
    Renderer::Renderer(const Vector2u& renderSize) : p_renderSize(renderSize) {
        // Set default state to start
        set_state(RenderState{}, true);
        resize(renderSize);

        // Initialize full-screen quad
        m_fullscreenQuad.create({
            StaticBuffer{{
                BufferAttribute{0, GL_FLOAT, 2, sizeof(Vector2f) * 2, 0},
                BufferAttribute{1, GL_FLOAT, 2, sizeof(Vector2f) * 2, sizeof(Vector2f)},
            }},
            StaticBuffer{{}, GL_ELEMENT_ARRAY_BUFFER}
        });
        m_fullscreenQuad.set_data(0, QUAD_VERTICES);
        m_fullscreenQuad.set_data(1, QUAD_INDICES);
    }

    // Functions
    void Renderer::resize(const Vector2u& size){
        p_renderSize = size;
    }

    void Renderer::begin_pass(AbstractRenderPass& pass){
        // Initialize this pass by setting the state
        assert(!m_currentPass && "Previous pass must be ended before starting another");
        m_currentPass = &pass;
    }

    void Renderer::end_pass(){
        assert(m_currentPass && "Cannot end a pass that has not started");
        m_currentPass = nullptr;
    }

    void Renderer::draw_fullscreen_quad() const {
        m_fullscreenQuad.bind();
        glDrawElements(GL_TRIANGLES, QUAD_INDICES.size(), GL_UNSIGNED_INT, nullptr);
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
        if(force || newState.cullFace) glCullFace(newState.cullMode);
        if(force || newState.frontFaceCCW != m_previousState.frontFaceCCW) glFrontFace(newState.frontFaceCCW ? GL_CCW : GL_CW);

        // Polygon offset
        if(force || newState.polygonOffset != m_previousState.polygonOffset) (newState.polygonOffset ? glEnable(GL_POLYGON_OFFSET_FILL) : glDisable(GL_POLYGON_OFFSET_FILL));

        // Scissor
        if(force || newState.scissor.has_value() != m_previousState.scissor.has_value()) (newState.scissor ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST));
        if(newState.scissor && (force || !m_previousState.scissor || newState.scissor->x != m_previousState.scissor->x || newState.scissor->y != m_previousState.scissor->y || newState.scissor->width != m_previousState.scissor->width || newState.scissor->height != m_previousState.scissor->height)){
            glScissor(newState.scissor->x, newState.scissor->y, newState.scissor->width, newState.scissor->height);
        }

        // Viewport
        if(newState.viewport){
            glViewport(newState.viewport->x, newState.viewport->y, newState.viewport->width, newState.viewport->height);
        } else {
            glViewport(0, 0, p_renderSize.x, p_renderSize.y);
        }

        // Clear color
        if(force || newState.clearColor != m_previousState.clearColor) glClearColor(newState.clearColor.x, newState.clearColor.y, newState.clearColor.z, newState.clearColor.w);

        // Make copy for keeping state tracked
        m_previousState = newState;
    }

    namespace {
        Resource<Shader> load_shader(const std::string& path){
            return Resource<Shader>(std::make_shared<AssetSlot<Shader>>(std::make_shared<Shader>(AssetFileSystem().open(path))));
        }
    }

    /// Default implementation
    DefaultRenderer::DefaultRenderer(const Vector2u& renderSize) :
        Renderer(renderSize),
        p_geometryShader(load_shader("assets/shaders/geometry")),
        p_compositeShader(load_shader("assets/shaders/composite")),
        p_interfaceShader(load_shader("assets/shaders/default")),
        p_geometryPass(p_geometryShader, renderSize),
        p_compositePass(p_compositeShader),
        p_interfacePass(p_interfaceShader)
    {
    }

    void DefaultRenderer::render_frame(Time dt, SystemRegistry& systems){
        // World rendering: submit, then flush
        systems.render_all(dt, RenderLayer::Geometry); // Fills the collections with data
        const Texture& geometry = p_geometryPass.run(*this); // Flushes the data
        p_compositePass.run(*this, geometry); // Composites the frames

        // Interface rendering: submit, then flush. The geometry flush above already drained
        // batch/shape, so this is a fresh queue InterfacePass alone drains
        systems.render_all(dt, RenderLayer::Interface);
        p_interfacePass.run(*this);

        // Anything that must draw after the whole pipeline above has finished (e.g. an
        // immediate-mode UI toolkit's own draw call, which CompositePass would otherwise erase)
        systems.render_all(dt, RenderLayer::Overlay);
    }

    void DefaultRenderer::resize(const Vector2u& size){
        Renderer::resize(size);
        p_geometryPass.resize(size);
    }
}
