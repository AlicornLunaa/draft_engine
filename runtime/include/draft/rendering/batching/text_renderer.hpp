#pragma once

#include "draft/asset/resource.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/shader.hpp"

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

    /**
     * @brief Draws text into a SpriteCollection, one instanced quad per glyph, via Font's baked
     * glyph atlas. Do not construct before an OpenGL context was established - the default font/
     * shader are lazily baked/compiled on first use.
     */
    class TextRenderer {
    public:
        // Constructors
        TextRenderer(Resource<Shader> shader = default_shader());
        ~TextRenderer() = default;

        // Functions
        inline void set_shader(Resource<Shader> shader){ fontShader = shader; }
        inline const Shader& get_shader() const { return *fontShader; }

        Vector2f get_text_bounds(const TextProperties& props) const;
        void draw_text(SpriteCollection& batch, const TextProperties& props);
        void draw_text(SpriteCollection& batch, const std::string& str, Font* font, const Vector2f& position, float scale = 1.f, const Vector4f& color = { 1, 1, 1, 1 });

    private:
        friend struct TextRendererTestAccess;

        // Functions
        static Resource<Shader> default_shader();
        const Font& get_font(const TextProperties& props) const;

        /**
         * @brief Computes a glyph's top-left draw position given the running horizontal cursor
         */
        static Vector2f compute_glyph_top_left(float currX, float positionY, const Vector2f& bearing, float scale, float stringHeight);

        // Variables
        Resource<Shader> fontShader;
    };
}
