#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/files/asset_file_system.hpp"

#include <algorithm>

namespace Draft {
    namespace {
        // Lazily-constructed, process-lifetime default font. Deliberately leaked via `new`.
        // See Material3D's debug-texture fallbacks (material.cpp) and ShapeCollection::default_shader() for the full reasoning.
        Font& default_font(){
            static Font* font = new Font(AssetFileSystem().open("assets/fonts/default.ttf"));
            return *font;
        }
    }

    // Static data
    Resource<Shader> TextRenderer::default_shader(){
        static std::shared_ptr<AssetSlot<Shader>>* slot = new std::shared_ptr<AssetSlot<Shader>>(
            std::make_shared<AssetSlot<Shader>>(
                std::make_shared<Shader>(AssetFileSystem().open("assets/shaders/text"))
            )
        );

        return Resource<Shader>(*slot);
    }

    // Private functions
    const Font& TextRenderer::get_font(const TextProperties& props) const {
        if(props.font)
            return *props.font;

        return default_font();
    }

    // Constructors
    TextRenderer::TextRenderer(Resource<Shader> shader) : fontShader(shader) {}

    // Functions
    Vector2f TextRenderer::compute_glyph_top_left(float currX, float positionY, const Vector2f& bearing, float scale, float stringHeight){
        return {
            currX + bearing.x * scale,
            positionY - bearing.y * scale + stringHeight
        };
    }

    Vector2f TextRenderer::get_text_bounds(const TextProperties& props) const {
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

    void TextRenderer::draw_text(SpriteCollection& batch, const TextProperties& props){
        Vector2f size = get_text_bounds(props);
        Vector2f center = size * props.origin;
        const Font& font = get_font(props);
        font.set_font_size(props.fontSize);

        float currX = props.position.x;

        for(char ch : props.str){
            // Get glyph to render
            auto& glyph = font.get_glyph(ch);

            Vector2f topLeft = compute_glyph_top_left(currX, props.position.y, glyph.bearing, props.scale, size.y);
            float w = glyph.size.x * props.scale;
            float h = glyph.size.y * props.scale;

            // Render glyph
            Material2D glyphMaterial;
            glyphMaterial.baseTexture = glyph.region.texture.get();
            glyphMaterial.transparent = true;
            glyphMaterial.tint = props.color;
            glyphMaterial.shader = fontShader.get();

            batch.draw(SpriteProps{
                topLeft,
                props.rotation,
                {w, h},
                center,
                0.f,
                glyph.region.bounds,
                glyphMaterial
            });

            currX += (glyph.advance >> 6) * props.scale;
        }
    }

    void TextRenderer::draw_text(SpriteCollection& batch, const std::string& str, Font* font, const Vector2f& position, float scale, const Vector4f& color){
        draw_text(batch, { str, font, 22, position, {0, 0}, color, 0.f, scale });
    }
}
