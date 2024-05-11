#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/sprite_batch.hpp"
#include "draft/util/asset_manager.hpp"

#include <string>

namespace Draft {
    struct TextProperties {
        std::string str = "Hello World!";
        const Font* font = nullptr;
        Vector2f position = {0, 0};
        Vector2f origin = {0, 0};
        Vector4f color = {1, 1, 1, 1};
        float rotation = 0.f;
        float scale = 1.f;
    };

    class TextRenderer {
    private:
        // Variables
        const Font& defaultFont = Assets::get_asset<Font>("assets/fonts/default.ttf");
        SpriteBatch batch;

        // Functions
        const Font& get_font(const TextProperties& props) const;

    public:
        // Constructors
        TextRenderer(const Shader& shader = Assets::get_asset<Shader>("assets/shaders/text"), size_t maxChars = 10000);

        // Functions
        Vector2f get_text_bounds(const TextProperties& props) const;
        void draw_text(const TextProperties& props);
        void draw_text(const std::string& str, const Font* font, const Vector2f& position, float scale = 1.f, const Vector4f& color = { 1, 1, 1, 1 });
        void flush(const RenderWindow& window, const Camera* camera = nullptr);
    };
};