#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"

#include <string>

namespace Draft {
    struct TextProperties {
        std::string str = "Hello World!";
        Font* font = nullptr;
        uint fontSize = 20;
        Vector2f position = {0, 0};
        Vector2f origin = {0, 0};
        Vector4f color = {1, 1, 1, 1};
        float rotation = 0.f;
        float scale = 1.f;
    };

    class TextRenderer {
    private:
        // Variables
        static StaticResource<Font> defaultFont;
        static StaticResource<Shader> defaultShader;
        Resource<Shader> fontShader;

        // Functions
        const Font& get_font(const TextProperties& props) const;

    public:
        // Constructors
        TextRenderer(Resource<Shader> shader = defaultShader);
        ~TextRenderer() = default;

        // Functions
        inline void set_shader(Resource<Shader> shader){ fontShader = shader; }
        inline const Shader& get_shader() const { return fontShader; }

        Vector2f get_text_bounds(const TextProperties& props) const;
        void draw_text(SpriteBatch& batch, const TextProperties& props);
        void draw_text(SpriteBatch& batch, const std::string& str, Font* font, const Vector2f& position, float scale = 1.f, const Vector4f& color = { 1, 1, 1, 1 });
    };
};