#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "glad/gl.h"
#include "glm/ext/matrix_transform.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

using namespace std;

namespace Draft {
    // Private functions
    void SpriteBatch::internal_flush_opaque(){
        // Flush all the opaque ones
        // Count the triangles, max of 2 * maxSprites
        size_t quadsRendered = std::min(quadQueue.size(), maxSprites);
        std::vector<InstanceData> instances;
        instances.reserve(quadsRendered);

        // Prep all the model matrices
        for(size_t i = 0; i < quadsRendered; i++){
            auto& props = quadQueue[i];

            auto& textureSize = props.texture->get_size();
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

            Matrix4 model(1.f);
            model *= Math::translate(Matrix4(1.f), Vector3f(props.position, 1.f));
            model *= Math::rotate(props.rotation, Vector3f(0.f, 0.f, 1.f));
            model *= Math::translate(Matrix4(1.f), Vector3f(-props.origin, 1.f));
            model *= Math::scale(Matrix4(1.f), Vector3f(props.size, 1.f));
            matrixArray.matrix[i] = model;
        }

        // Prep the shader
        shaderPtr->bind();
        shaderPtr->set_uniform("view", get_trans_matrix());
        shaderPtr->set_uniform("projection", get_proj_matrix());

        // Render the opaque instances
        shaderBuffer.bind();
        shaderBuffer.set(matrixArray);

        vertexBuffer.bind();
        vertexBuffer.set_dynamic_data(dynamicDataLoc, instances);

        // Render every triangle
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, quadsRendered);

        // Clear all the triangles rendered
        quadQueue.clear();
    }

    void SpriteBatch::internal_flush_transparent(){
        // Flush all the transparent quads
        if(transparentQuads.empty())
            return; // Exit early

        // Count the triangles, max of 2 * maxSprites
        size_t quadsRendered = std::min(transparentQuads.size(), maxSprites);
        size_t currentModel = 0;
        std::vector<InstanceData> instances;
        instances.reserve(quadsRendered);

        // Prep all the model matrices
        while(!transparentQuads.empty()){
            auto& props = transparentQuads.top();

            auto& textureSize = props.texture->get_size();
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

            Matrix4 model(1.f);
            model *= Math::translate(Matrix4(1.f), Vector3f(props.position, 1.f));
            model *= Math::rotate(props.rotation, Vector3f(0.f, 0.f, 1.f));
            model *= Math::translate(Matrix4(1.f), Vector3f(-props.origin, 1.f));
            model *= Math::scale(Matrix4(1.f), Vector3f(props.size, 1.f));
            matrixArray.matrix[currentModel++] = model;

            transparentQuads.pop();
        }

        // Prep the shader
        shaderPtr->bind();
        shaderPtr->set_uniform("view", get_trans_matrix());
        shaderPtr->set_uniform("projection", get_proj_matrix());

        // Render the opaque instances
        shaderBuffer.bind();
        shaderBuffer.set(matrixArray);

        vertexBuffer.bind();
        vertexBuffer.set_dynamic_data(dynamicDataLoc, instances);

        // Render every triangle
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, quadsRendered);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }

    // Constructor
    SpriteBatch::SpriteBatch(std::shared_ptr<Shader> shader, const size_t maxSprites) : Batch(maxSprites, shader) {
        // Buffer the data on the GPU
        vertexBuffer.buffer(0, QUAD_VERTICES);
        vertexBuffer.buffer(3, QUAD_INDICES, GL_ELEMENT_ARRAY_BUFFER);

        dynamicDataLoc = vertexBuffer.start_buffer<InstanceData>(maxSprites);
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

        quadQueue.reserve(maxSprites);
    }

    // Functions
    void SpriteBatch::draw(SpriteProps props){
        // Preprocessing for the props
        if(!props.texture){
            // No texture means use debug white
            props.texture = whiteTexture;
        }

        // Check texture, if its changed flush immediately
        if(props.texture != previousTexture){
            internal_flush_opaque();
            previousTexture = props.texture;
            props.texture->bind();
        }

        // Flush immediately if there are too many sprites
        if(quadQueue.size() >= maxSprites){
            internal_flush_opaque();
        }

        // Check if the sprite is translucent or not
        if(props.color.a < 1.f || props.texture->is_transparent() || props.renderAsTransparent){
            // translucent sprite, save sprite information and recreate it at runtime for the buffer
            transparentQuads.push(props);
        } else {
            // Clean sprite, fully opaque. Store its render information immediately
            quadQueue.push_back(props);
        }
    }

    void SpriteBatch::draw(const std::shared_ptr<Texture> texture, const Vector2f& position, const Vector2f& size, float rotation, const Vector2f& origin, FloatRect region){
        // Shortcut function for backwards compat
        draw({
            texture,
            position,
            rotation,
            size,
            origin,
            0,
            region
        });
    }

    void SpriteBatch::begin(){
        // Save the current window so flushing can happen
        previousTexture = nullptr;
    }

    void SpriteBatch::flush(){
        // Draws all the shapes to opengl
        if(quadQueue.empty() && transparentQuads.empty())
            return;

        // Flush first batch of opaque & transparent remaining
        internal_flush_opaque();
        internal_flush_transparent();
    }
};