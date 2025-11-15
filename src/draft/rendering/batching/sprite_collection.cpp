#include "draft/math/glm.hpp"
#include "draft/rendering/batching/collection.hpp"
#include "draft/rendering/batching/draw_command.hpp"
#include "draft/rendering/batching/sprite_props.hpp"
#include "draft/rendering/material.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/rendering/batching/sprite_collection.hpp"
#include "glad/gl.h"
#include <optional>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include <cstddef>
#include <vector>

using namespace std;

namespace Draft {
    // Static data
    StaticResource<Shader> SpriteCollection::s_defaultShader = {FileHandle("assets/shaders/default")};

    // Helpers
    template<QueueLike Q>
    auto get_front(Q& q){
        if constexpr (requires { q.front(); }){
            return q.front();
        } else {
            return q.top();
        }
    };

    // Private functions
    void SpriteCollection::flush_generic(QueueLike auto& queue){
        // Flush all the queued quads to gpu
        std::optional<Material2D> material;
        std::vector<InstanceData> instances;
        instances.reserve(MAX_SPRITES_TO_RENDER);

        // Prep buffer
        m_vertexBuffer.bind();

        // Assemble quads and render them in chunks of maxSprites
        while(!queue.empty()){
            material.reset();

            for(size_t i = 0; i < MAX_SPRITES_TO_RENDER && !queue.empty(); i++){
                const SpriteDrawCommand& command = get_front(queue);
                const SpriteProps& sprite = command.sprite;

                if(!material.has_value()){
                    // No material previously, apply it
                    material = sprite.material;
                    material->apply();
                    material->shader->set_uniform("view", command.transformMatrix);
                    material->shader->set_uniform("projection", command.projectionMatrix);
                }

                if(command.matricesDirty){
                    material->shader->set_uniform("view", command.transformMatrix);
                    material->shader->set_uniform("projection", command.projectionMatrix);
                }

                if(!(sprite.material == *material)){
                    // Different material, escape this loop to trigger an immediate flush and restart
                    material.reset();
                    break;
                }

                auto textureSize = material->baseTexture->get_properties().size;
                float x = sprite.textureRegion.x / textureSize.x;
                float y = sprite.textureRegion.y / textureSize.y;
                float w = ((sprite.textureRegion.width <= 0) ? textureSize.x : sprite.textureRegion.width) / textureSize.x;
                float h = ((sprite.textureRegion.height <= 0) ? textureSize.y : sprite.textureRegion.height) / textureSize.y;

                instances.push_back({
                    sprite.material.tint,
                    {
                        {x, y},
                        {x + w, y},
                        {x + w, y + h},
                        {x, y + h}
                    },
                });

                matrixArray.matrices[instances.size() - 1] = Optimal::fast_model_matrix(sprite.position, sprite.rotation, sprite.size, sprite.origin, sprite.zIndex);
                queue.pop();
            }

            // Render the opaque instances
            m_shaderBuffer.set(matrixArray);
            m_vertexBuffer.set_dynamic_data(m_dynamicDataLoc, instances);

            // Render every triangle
            glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instances.size());

            // Reset instances
            instances.clear();
        }
    }

    // Constructor
    SpriteCollection::SpriteCollection() : Collection() {
        // Profiling
        ZoneScopedN("sprite_batch_setup");

        // Buffer the data on the GPU
        m_vertexBuffer.buffer(0, QUAD_VERTICES);
        m_vertexBuffer.buffer(3, QUAD_INDICES, GL_ELEMENT_ARRAY_BUFFER);

        m_dynamicDataLoc = m_vertexBuffer.start_buffer<InstanceData>(MAX_SPRITES_TO_RENDER);
        m_vertexBuffer.set_attribute(1, GL_FLOAT, 4, sizeof(InstanceData), offsetof(InstanceData, color));
        m_vertexBuffer.set_attribute(2, GL_FLOAT, 2, sizeof(InstanceData), offsetof(InstanceData, texCoords));
        m_vertexBuffer.set_attribute(3, GL_FLOAT, 2, sizeof(InstanceData), offsetof(InstanceData, texCoords) + sizeof(Vector2f) * 1);
        m_vertexBuffer.set_attribute(4, GL_FLOAT, 2, sizeof(InstanceData), offsetof(InstanceData, texCoords) + sizeof(Vector2f) * 2);
        m_vertexBuffer.set_attribute(5, GL_FLOAT, 2, sizeof(InstanceData), offsetof(InstanceData, texCoords) + sizeof(Vector2f) * 3);
        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        m_vertexBuffer.end_buffer();
    }
    
    // Functions
    void SpriteCollection::draw(SpriteProps props){
        // Profiling
        ZoneScopedN("sprite_batch_draw");

        // Preprocessing for the props
        if(!props.material.shader){
            props.material.shader = s_defaultShader;
        }

        if(!props.material.baseTexture){
            props.material.baseTexture = &whiteTexture;
        }

        if(!props.material.normalTexture){
            props.material.normalTexture = &normalTexture;
        }

        if(!props.material.emissiveTexture){
            props.material.emissiveTexture = &blackTexture;
        }

        // Check if the sprite is translucent or not
        if(props.material.tint.a < 1.f || props.material.baseTexture->get_properties().transparent || props.material.transparent){
            // translucent sprite, save sprite information and recreate it at runtime for the buffer
            m_transparentQuads.push({
                props,
                projMatrix,
                transMatrix,
                p_matricesDirty
            });
        } else {
            // Clean sprite, fully opaque. Store its render information immediately
            m_opaqueQuads.push({
                props,
                projMatrix,
                transMatrix,
                p_matricesDirty
            });
        }

        p_matricesDirty = false;
    }

    void SpriteCollection::flush(){
        // Profiling
        ZoneScopedN("sprite_batch_flush");
        TracyGpuZone("sprite_batch");

        // Draws all the shapes to opengl
        if(m_opaqueQuads.empty() && m_transparentQuads.empty())
            return;

        // Flush first batch of opaque & transparent remaining
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        flush_opaque();

        // Set render options
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        flush_transparent();

        // Restore opengl
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }

    void SpriteCollection::flush_opaque(){
        // Flush all the opaque quads to gpu
        flush_generic(m_opaqueQuads);
    }

    void SpriteCollection::flush_transparent(){
        // Flush all the transparent quads
        flush_generic(m_transparentQuads);
    }
};