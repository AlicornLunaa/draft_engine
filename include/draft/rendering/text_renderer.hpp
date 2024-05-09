#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/asset_manager.hpp"
#include <queue>
#include <string>

namespace Draft {
    class TextRenderer {
    private:
        // Structures
        struct Text {
            const Font& font;
            Vector2f position;
            std::string str;
            Vector4f color;
        };

        // Variables
        const size_t maxChars;
        const Shader& shader;
        std::queue<Text> textQueue;

        VertexBuffer vertexBuffer;
        size_t dynamicBufLoc;

    public:
        // Constructors
        TextRenderer(const Shader& shader = Assets::get_asset<Shader>("assets/shaders/text"), size_t maxChars = 1000);

        // Functions
        void draw_text(const std::string& str, const Font& font, const Vector2f& position, const Vector4f& color = { 1, 1, 1, 1 });
        void flush(const RenderWindow& window, const Camera* camera = nullptr);
    };
};