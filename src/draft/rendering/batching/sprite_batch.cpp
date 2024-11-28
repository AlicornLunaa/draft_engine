#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "glad/gl.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include <cstddef>
#include <vector>

using namespace std;

namespace Draft {
    // Private functions
    void SpriteBatch::internal_flush_opaque(){
        // Flush all the opaque quads to gpu
        Texture const* texturePtr = nullptr;
        std::vector<InstanceData> instances;
        instances.reserve(MAX_SPRITES_TO_RENDER);

        // Prep the shader
        Shader* shader = this->shader;
        shader->bind();
        shader->set_uniform("view", get_trans_matrix());
        shader->set_uniform("projection", get_proj_matrix());

        // Prep buffer
        vertexBuffer.bind();

        // Assemble quads and render them in chunks of maxSprites
        while(!opaqueQuads.empty()){
            texturePtr = nullptr;

            for(size_t i = 0; i < MAX_SPRITES_TO_RENDER && !opaqueQuads.empty(); i++){
                auto& props = opaqueQuads.front();

                if(!texturePtr){
                    // No texture previously, bind it
                    texturePtr = props.texture;
                    texturePtr->bind();
                }

                if(props.texture != texturePtr){
                    // Different texture, bind it and flush immediately
                    texturePtr = nullptr;
                    break;
                }

                auto textureSize = props.texture->get_size();
                float x = props.region.x / textureSize.x;
                float y = props.region.y / textureSize.y;
                float w = ((props.region.width <= 0) ? textureSize.x : props.region.width) / textureSize.x;
                float h = ((props.region.height <= 0) ? textureSize.y : props.region.height) / textureSize.y;

                instances.push_back({
                    props.color,
                    {
                        {x, y},
                        {x + w, y},
                        {x + w, y + h},
                        {x, y + h}
                    },
                });

                matrixArray.matrices[instances.size() - 1] = Optimal::fast_model_matrix(props.position, props.rotation, props.size, props.origin, props.zIndex);

                opaqueQuads.pop();
            }

            // Render the opaque instances
            shaderBuffer.set(matrixArray);
            vertexBuffer.set_dynamic_data(dynamicDataLoc, instances);

            // Render every triangle
            glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instances.size());

            // Reset instances
            instances.clear();
        }
    }

    void SpriteBatch::internal_flush_transparent(){
        // Flush all the transparent quads
        Texture const* texturePtr = nullptr;
        std::vector<InstanceData> instances;
        instances.reserve(MAX_SPRITES_TO_RENDER);

        // Prep the shader
        Shader* shader = this->shader;
        shader->bind();
        shader->set_uniform("view", get_trans_matrix());
        shader->set_uniform("projection", get_proj_matrix());

        // Prep buffer
        vertexBuffer.bind();

        // Set render options
        if(blend){ glEnable(GL_BLEND); } else { glDisable(GL_BLEND); }
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Render in maxSprites chunks
        while(!transparentQuads.empty()){
            texturePtr = nullptr;

            for(size_t i = 0; i < MAX_SPRITES_TO_RENDER && !transparentQuads.empty(); i++){
                auto& props = transparentQuads.top();

                if(!texturePtr){
                    // No texture previously, bind it
                    texturePtr = props.texture;
                    texturePtr->bind();
                }

                if(props.texture != texturePtr){
                    // Different texture, flush immediately
                    texturePtr = nullptr;
                    break;
                }

                auto textureSize = props.texture->get_size();
                float x = props.region.x / textureSize.x;
                float y = props.region.y / textureSize.y;
                float w = ((props.region.width <= 0) ? textureSize.x : props.region.width) / textureSize.x;
                float h = ((props.region.height <= 0) ? textureSize.y : props.region.height) / textureSize.y;

                instances.push_back({
                    props.color,
                    {
                        {x, y},
                        {x + w, y},
                        {x + w, y + h},
                        {x, y + h}
                    },
                });

                matrixArray.matrices[instances.size() - 1] = Optimal::fast_model_matrix(props.position, props.rotation, props.size, props.origin, props.zIndex);

                transparentQuads.pop();
            }

            // Render the opaque instances
            shaderBuffer.set(matrixArray);
            vertexBuffer.set_dynamic_data(dynamicDataLoc, instances);

            // Render every triangle
            glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instances.size());

            // Remove maxSprites from queue
            instances.clear();
        }

        // Restore opengl
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }

    // Constructor
    SpriteBatch::SpriteBatch(Resource<Shader> shader) : Batch(shader) {
        // Profiling
        ZoneScopedN("sprite_batch_setup");

        // Buffer the data on the GPU
        vertexBuffer.buffer(0, QUAD_VERTICES);
        vertexBuffer.buffer(3, QUAD_INDICES, GL_ELEMENT_ARRAY_BUFFER);

        dynamicDataLoc = vertexBuffer.start_buffer<InstanceData>(MAX_SPRITES_TO_RENDER);
        vertexBuffer.set_attribute(1, GL_FLOAT, 4, sizeof(InstanceData), offsetof(InstanceData, color));
        vertexBuffer.set_attribute(2, GL_FLOAT, 2, sizeof(InstanceData), offsetof(InstanceData, texCoords));
        vertexBuffer.set_attribute(3, GL_FLOAT, 2, sizeof(InstanceData), offsetof(InstanceData, texCoords) + sizeof(Vector2f) * 1);
        vertexBuffer.set_attribute(4, GL_FLOAT, 2, sizeof(InstanceData), offsetof(InstanceData, texCoords) + sizeof(Vector2f) * 2);
        vertexBuffer.set_attribute(5, GL_FLOAT, 2, sizeof(InstanceData), offsetof(InstanceData, texCoords) + sizeof(Vector2f) * 3);
        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        vertexBuffer.end_buffer();
    }
    
    // Functions
    void SpriteBatch::draw(SpriteProps props){
        // Profiling
        ZoneScopedN("sprite_batch_draw");

        // Preprocessing for the props
        if(!props.texture){
            // No texture means use debug white
            props.texture = &whiteTexture;
        }

        // Check if the sprite is translucent or not
        if(props.color.a < 1.f || props.texture->is_transparent() || props.renderAsTransparent){
            // translucent sprite, save sprite information and recreate it at runtime for the buffer
            transparentQuads.push(props);
        } else {
            // Clean sprite, fully opaque. Store its render information immediately
            opaqueQuads.push(props);
        }
    }

    void SpriteBatch::draw(Texture* texture, const Vector2f& position, const Vector2f& size, float rotation, const Vector2f& origin, FloatRect region){
        // Shortcut function for backwards compat
        draw(SpriteProps{
            texture,
            region,
            position,
            rotation,
            size,
            origin,
            0
        });
    }

    void SpriteBatch::begin(){
        // Profiling
        ZoneScopedN("sprite_batch_begin");
        
        // Prep for rendering
        Batch::begin();

        while(!transparentQuads.empty())
            transparentQuads.pop();

        while(!opaqueQuads.empty())
            opaqueQuads.pop();
    }

    void SpriteBatch::flush(){
        // Profiling
        ZoneScopedN("sprite_batch_flush");
        TracyGpuZone("sprite_batch");

        // Draws all the shapes to opengl
        if(opaqueQuads.empty() && transparentQuads.empty())
            return;

        // Flush first batch of opaque & transparent remaining
        internal_flush_opaque();
        internal_flush_transparent();
    }
    
    void SpriteBatch::end(){
        // Profiling
        ZoneScopedN("sprite_batch_end");

        // Finalize rendering
        Batch::end();
    }
};