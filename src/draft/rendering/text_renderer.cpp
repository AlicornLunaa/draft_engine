#include "draft/rendering/text_renderer.hpp"
#include "draft/math/glm.hpp"
#include "glad/gl.h"
#include <vector>

namespace Draft {
    // Constructors
    TextRenderer::TextRenderer(const Shader& shader, size_t maxChars) : shader(shader), maxChars(maxChars) {
        dynamicBufLoc = vertexBuffer.start_buffer<Vector4f>(maxChars * 4);
        vertexBuffer.set_attribute(0, GL_FLOAT, 4, sizeof(Vector4f), 0);
        vertexBuffer.end_buffer();
    }

    // Functions
    void TextRenderer::draw_text(const std::string& str, const Font& font, const Vector2f& position, const Vector4f& color){
        Text text{
            font,
            position,
            str,
            color
        };
        textQueue.emplace(text);
    }

    void TextRenderer::flush(const RenderWindow& window, const Camera* camera){
        // Generate every quad for each character
        bool flushAgain = false;
        float scale = 0.01f;
        size_t characterCount = 0;
        std::vector<Vector4f> vertices{6};

        if(textQueue.empty())
            return;

        shader.bind();

        if(camera)
            camera->apply(window, shader);

        // Loop over each text to render
        while(!textQueue.empty()){
            // Get the next item
            auto& text = textQueue.front();
            float currX = text.position.x;
            shader.set_uniform("textColor", text.color);

            // Create quad coordinates for each char in the text
            for(char ch : text.str){
                // Get glyph to render
                auto& glyph = text.font.get_glyph(ch);
                
                float xPos = currX + glyph.bearing.x * scale;
                float yPos = text.position.y - (glyph.size.y - glyph.bearing.y) * scale;
                float w = glyph.size.x * scale;
                float h = glyph.size.y * scale;

                // Update VBO data
                vertices[0] = { xPos, yPos + h, 0.0f, 0.0f };         
                vertices[1] = { xPos, yPos, 0.0f, 1.0f };
                vertices[2] = { xPos + w, yPos, 1.0f, 1.0f };
                vertices[3] = { xPos, yPos + h, 0.0f, 0.0f };
                vertices[4] = { xPos + w, yPos, 1.0f, 1.0f };
                vertices[5] = { xPos + w, yPos + h, 1.0f, 0.0f };
                vertexBuffer.set_dynamic_data(dynamicBufLoc, vertices);

                // Render this glyph
                vertexBuffer.bind();
                glyph.texture->bind();
                glDrawArrays(GL_TRIANGLES, 0, vertices.size());
                vertexBuffer.unbind();

                // Advance
                currX += (glyph.advance >> 6) * scale;
            }

            // Remove current text
            textQueue.pop();

            // Check char count
            // if(++characterCount >= maxChars){
            //     flushAgain = true;
            //     break;
            // }
        }
    }
};