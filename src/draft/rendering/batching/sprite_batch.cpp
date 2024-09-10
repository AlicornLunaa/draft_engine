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
    // void SpriteBatch::assemble_quad(std::vector<Quad>& vertices, std::vector<int>& indices, std::queue<std::pair<std::shared_ptr<Texture>, size_t>>& textureRegister, const SpriteProps& props){
    //     // Create quad data
    //     size_t index = vertices.size();

    //     // Obtain texture ID in order to pool textures without rebinding every quad
    //     if(!textureRegister.empty() && textureRegister.back().first == props.texture){
    //         // Current texture is the same texture in the queue, increase the quad count
    //         textureRegister.back().second++;
    //     } else {
    //         // Current texture is different, add new texture
    //         textureRegister.push({props.texture, 1});
    //     }

    //     // Create texture coord data
    //     auto& textureSize = props.texture->get_size();
    //     float x = props.region.x / textureSize.x;
    //     float y = props.region.y / textureSize.y;
    //     float w = ((props.region.width <= 0) ? textureSize.x : props.region.width) / textureSize.x;
    //     float h = ((props.region.height <= 0) ? textureSize.y : props.region.height) / textureSize.y;

    //     // Create vertices, TODO: Texture coords based on region
    //     vertices.push_back({ {0, 0, props.zIndex}, {x, y}, props.color });
    //     vertices.push_back({ {1, 0, props.zIndex}, {x + w, y}, props.color });
    //     vertices.push_back({ {1, 1, props.zIndex}, {x + w, y + h}, props.color });
    //     vertices.push_back({ {0, 1, props.zIndex}, {x, y + h}, props.color });

    //     // Rotate quad positions if needed
    //     auto rotate_vertex = [props](Vector3f& v){
    //         v *= Vector3f{props.size, 1.f};
    //         v -= Vector3f{props.origin, 0.f};

    //         if(props.rotation != 0)
    //             v = Vector3f(Math::rotate(Matrix3(1.f), props.rotation) * v);

    //         v += Vector3f{props.position, 0.f};
    //     };
    //     rotate_vertex(vertices[index].position);
    //     rotate_vertex(vertices[index + 1].position);
    //     rotate_vertex(vertices[index + 2].position);
    //     rotate_vertex(vertices[index + 3].position);

    //     // Create indices
    //     indices.push_back(1 + index);
    //     indices.push_back(0 + index);
    //     indices.push_back(3 + index);
    //     indices.push_back(1 + index);
    //     indices.push_back(3 + index);
    //     indices.push_back(2 + index);
    // }

    // void SpriteBatch::flush_batch_internal(){
        // Loops through the texture register and renders the quads
        // while(!textureRegister.empty()){
        //     // Get data
        //     auto& [texture, quadCount] = textureRegister.front();
        //     size_t maxQuads = std::min(quadCount, maxSprites);

        //     // Buffer data
        //     // vertexBuffer.set_dynamic_data(dynamicVertexBufLoc, vertices);
        //     // vertexBuffer.set_dynamic_data(dynamicIndexBufLoc, indices);

        //     // Render all the vertices
        //     texture->bind();
        //     vertexBuffer.bind();
        //     glDrawElements(GL_TRIANGLES, maxQuads * 6, GL_UNSIGNED_INT, 0);
        //     vertexBuffer.unbind();

        //     // Cleanup used vertices and prep for next iteration for textures
        //     // vertices.erase(vertices.begin(), vertices.begin() + maxQuads * 4);
        //     // indices.erase(indices.begin(), indices.begin() + maxQuads * 6);

        //     // // Shift indices for the deleted vertices
        //     // for(auto& index : indices){
        //     //     index -= maxQuads * 4;
        //     // }

        //     // Check if the vertices were completed
        //     if(quadCount > maxSprites){
        //         // Run again
        //         textureRegister.front().second -= maxQuads;
        //     } else {
        //         // Remove this iteration
        //         textureRegister.pop();
        //     }
        // }
    // }

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

    void SpriteBatch::begin(const RenderWindow& window){
        // Save the current window so flushing can happen
    }

    void SpriteBatch::flush(const RenderWindow& window){
        // Draws all the shapes to opengl
        if(quadQueue.empty() && transparentQuads.empty())
            return;

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
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, quadsRendered * 2);

        // Clear all the triangles rendered
        quadQueue.clear();

        // Exit early
        if(transparentQuads.empty()){
            // vertices.clear();
            // indices.clear();
            return;
        }

        // After the opaque textures were rendered, start on the transparent ones
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // vertices.clear();
        // indices.clear();

        // Generate quad data for transparents
        while(!transparentQuads.empty()){
            // assemble_quad(vertices, indices, textureRegister, transparentQuads.top());
            transparentQuads.pop();
        }

        // Flush again for the new transparent data
        // flush_batch_internal();

        // Back to normal rendering
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        // Clear out data for next flush
        // vertices.clear();
        // indices.clear();
    }
};