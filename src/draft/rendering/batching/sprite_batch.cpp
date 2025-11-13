#include "draft/math/glm.hpp"
#include "draft/rendering/material.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "glad/gl.h"
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include <cstddef>
#include <vector>

using namespace std;

namespace Draft {
    // Static data
    StaticResource<Shader> SpriteBatch::defaultShader = {FileHandle("assets/shaders/default")};

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
        if(!props.material.shader){
            // No texture means use debug white
            props.material.shader = defaultShader;
        }

        if(!props.material.baseTexture){
            // No texture means use debug white
            props.material.baseTexture = &whiteTexture;
        }

        // Check if the sprite is translucent or not
        if(props.material.tint.a < 1.f || props.material.baseTexture->get_properties().transparent || props.material.transparent){
            // translucent sprite, save sprite information and recreate it at runtime for the buffer
            transparentQuads.push(props);
        } else {
            // Clean sprite, fully opaque. Store its render information immediately
            opaqueQuads.push(props);
        }
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
        flush_opaque();

        // Set render options
        if(blend){ glEnable(GL_BLEND); } else { glDisable(GL_BLEND); }
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        flush_transparent();

        // Restore opengl
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }

    void SpriteBatch::flush_opaque(){
        // Flush all the opaque quads to gpu
        Material2D const* materialPtr = nullptr;

        std::vector<InstanceData> instances;
        instances.reserve(MAX_SPRITES_TO_RENDER);

        // Prep buffer
        vertexBuffer.bind();

        // Assemble quads and render them in chunks of maxSprites
        while(!opaqueQuads.empty()){
            materialPtr = nullptr;

            for(size_t i = 0; i < MAX_SPRITES_TO_RENDER && !opaqueQuads.empty(); i++){
                auto& props = opaqueQuads.front();

                if(!materialPtr){
                    // No material previously, apply it
                    materialPtr = &props.material;                    
                    materialPtr->shader->set_uniform("projection", get_proj_matrix());
                    materialPtr->shader->set_uniform("view", get_trans_matrix());
                    materialPtr->apply();
                }

                if(props.material != *materialPtr){
                    // Different material, escape this loop to trigger an immediate flush and restart
                    materialPtr = nullptr;
                    break;
                }

                auto textureSize = props.material.baseTexture->get_properties().size;
                float x = props.textureRegion.x / textureSize.x;
                float y = props.textureRegion.y / textureSize.y;
                float w = ((props.textureRegion.width <= 0) ? textureSize.x : props.textureRegion.width) / textureSize.x;
                float h = ((props.textureRegion.height <= 0) ? textureSize.y : props.textureRegion.height) / textureSize.y;

                instances.push_back({
                    props.material.tint,
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

    void SpriteBatch::flush_transparent(){
        // Flush all the transparent quads
        Material2D const* materialPtr = nullptr;

        std::vector<InstanceData> instances;
        instances.reserve(MAX_SPRITES_TO_RENDER);

        // Prep the shader
        Shader* shader = this->shader;
        shader->bind();
        shader->set_uniform("view", get_trans_matrix());
        shader->set_uniform("projection", get_proj_matrix());

        // Prep buffer
        vertexBuffer.bind();

        // Render in maxSprites chunks
        while(!transparentQuads.empty()){
            materialPtr = nullptr;

            for(size_t i = 0; i < MAX_SPRITES_TO_RENDER && !transparentQuads.empty(); i++){
                auto& props = transparentQuads.top();

                if(!materialPtr){
                    // No material previously, apply it
                    materialPtr = &props.material;
                    materialPtr->shader->set_uniform("projection", get_proj_matrix());
                    materialPtr->shader->set_uniform("view", get_trans_matrix());
                    materialPtr->apply();
                }

                if(props.material != *materialPtr){
                    // Different material, escape this loop to trigger an immediate flush and restart
                    materialPtr = nullptr;
                    break;
                }

                auto textureSize = props.material.baseTexture->get_properties().size;
                float x = props.textureRegion.x / textureSize.x;
                float y = props.textureRegion.y / textureSize.y;
                float w = ((props.textureRegion.width <= 0) ? textureSize.x : props.textureRegion.width) / textureSize.x;
                float h = ((props.textureRegion.height <= 0) ? textureSize.y : props.textureRegion.height) / textureSize.y;

                instances.push_back({
                    props.material.tint,
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
    }
    
    void SpriteBatch::end(){
        // Profiling
        ZoneScopedN("sprite_batch_end");

        // Finalize rendering
        Batch::end();
    }
};