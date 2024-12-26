#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/math/glm.hpp"
#include "tracy/Tracy.hpp"
#include <algorithm>

namespace Draft {
    // Private functions
    const Font& TextRenderer::get_font(const TextProperties& props) const {
        if(props.font)
            return *props.font;

        return defaultFont;
    }

    // Constructors
    TextRenderer::TextRenderer(Resource<Shader> shader) : fontShader(shader) {}

    // Functions
    Vector2f TextRenderer::get_text_bounds(const TextProperties& props) const {
        // Profiling
        ZoneScopedN("text_renderer_get_bounds");

        const Font& font = get_font(props);
        font.set_font_size(props.fontSize);

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

    void TextRenderer::draw_text(SpriteBatch& batch, const TextProperties& props){
        // Profiling
        ZoneScopedN("text_renderer_draw");

        Vector2f size = get_text_bounds(props);
        Vector2f center = size * props.origin;
        const Font& font = get_font(props);
        font.set_font_size(props.fontSize);

        float currX = props.position.x;
        batch.set_shader(fontShader);

        for(char ch : props.str){
            // Get glyph to render
            auto& glyph = font.get_glyph(ch);
            Vector2f glyphTextureSize = glyph.region.texture.get().get_properties().size;
            
            float xPos = currX + glyph.bearing.x * props.scale;
            float yPos = props.position.y - (glyph.size.y - glyph.bearing.y) * props.scale;
            float w = glyph.size.x * props.scale;
            float h = glyph.size.y * props.scale;

            // Render glyph
            batch.draw(SpriteProps{
                glyph.region.texture,
                glyph.region.bounds,
                Vector2f{xPos, yPos},
                props.rotation,
                {w, h},
                center,
                0.f,
                props.color,
                true
            });
            
            currX += (glyph.advance >> 6) * props.scale;
        }
    }

    void TextRenderer::draw_text(SpriteBatch& batch, const std::string& str, Font* font, const Vector2f& position, float scale, const Vector4f& color){
        draw_text(batch, { str, font, 22, position, {0, 0}, color, 0.f, scale });
    }
};