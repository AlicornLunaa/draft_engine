#include "draft/rendering/text_renderer.hpp"
#include "draft/math/glm.hpp"
#include <algorithm>

namespace Draft {
    // Private functions
    const Font& TextRenderer::get_font(const TextProperties& props) const {
        if(props.font)
            return *props.font;

        return defaultFont;
    }

    // Constructors
    TextRenderer::TextRenderer(const Shader& shader, size_t maxChars) : batch(shader, maxChars) {}

    // Functions
    Vector2f TextRenderer::get_text_bounds(const TextProperties& props) const {
        const Font& font = get_font(props);
        float currWidth = 0.f;
        float currHeight = 0.f;

        for(char ch : props.str){
            // Get glyph to render
            auto& glyph = font.get_glyph(ch);
            currWidth += (glyph.advance >> 6) * props.scale;
            currHeight = std::max(glyph.size.y * props.scale, currHeight);
        }

        return {currWidth, currHeight};
    }

    void TextRenderer::draw_text(const TextProperties& props){
        Vector2f size = get_text_bounds(props);
        Vector2f center = size * props.origin;
        const Font& font = get_font(props);
        float currX = props.position.x;

        batch.set_color(props.color);

        for(char ch : props.str){
            // Get glyph to render
            auto& glyph = font.get_glyph(ch);
            Vector2f glyphTextureSize = glyph.region.texture.get_size();
            
            float xPos = currX + glyph.bearing.x * props.scale;
            float yPos = props.position.y - (glyph.size.y - glyph.bearing.y) * props.scale;
            float w = glyph.size.x * props.scale;
            float h = glyph.size.y * props.scale;

            // Render glyph
            batch.draw(
                glyph.region.texture,
                {0, 0},
                {w, h},
                props.rotation,
                center - Vector2f(xPos, yPos),
                glyph.region.bounds
            );
            
            currX += (glyph.advance >> 6) * props.scale;
        }
    }

    void TextRenderer::draw_text(const std::string& str, const Font* font, const Vector2f& position, float scale, const Vector4f& color){
        draw_text({ str, font, position, {0, 0}, color, 0.f, scale });
    }

    void TextRenderer::flush(const RenderWindow& window, const Camera* camera){
        // Generate every quad for each character
        batch.flush(window, camera);
    }
};