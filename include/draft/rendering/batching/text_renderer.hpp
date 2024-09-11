#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/util/asset_manager.hpp"

#include <memory>
#include <string>

namespace Draft {
    struct TextProperties {
        std::string str = "Hello World!";
        std::shared_ptr<Font> font = nullptr;
        Vector2f position = {0, 0};
        Vector2f origin = {0, 0};
        Vector4f color = {1, 1, 1, 1};
        float rotation = 0.f;
        float scale = 1.f;
    };

    class TextRenderer {
    private:
        // Variables
        const std::shared_ptr<Font> defaultFont = Assets::manager.get<Font>("assets/fonts/default.ttf", true);
        SpriteBatch batch;

        // Functions
        const Font& get_font(const TextProperties& props) const;

    public:
        // Constructors
        TextRenderer(const std::shared_ptr<Shader> shader = Assets::manager.get<Shader>("assets/shaders/text", true), size_t maxChars = 10000);

        // Functions
        Vector2f get_text_bounds(const TextProperties& props) const;
        void draw_text(const TextProperties& props);
        void draw_text(const std::string& str, std::shared_ptr<Font> font, const Vector2f& position, float scale = 1.f, const Vector4f& color = { 1, 1, 1, 1 });
        void flush(const Camera* camera = nullptr);
    };
};