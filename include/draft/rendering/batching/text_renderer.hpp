#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"

#include <string>

namespace Draft {
    struct TextProperties {
        std::string str = "Hello World!";
        Font* font = nullptr;
        Vector2f position = {0, 0};
        Vector2f origin = {0, 0};
        Vector4f color = {1, 1, 1, 1};
        float rotation = 0.f;
        float scale = 1.f;
    };

    class TextRenderer {
    private:
        // Variables
        const Resource<Font> defaultFont = Assets::manager.get<Font>("assets/fonts/default.ttf", true);
        SpriteBatch batch;

        // Functions
        const Font& get_font(const TextProperties& props) const;

    public:
        // Constructors
        TextRenderer(Resource<Shader> shader = Assets::manager.get<Shader>("assets/shaders/text", true));
        virtual ~TextRenderer() = default;

        // Functions
        Vector2f get_text_bounds(const TextProperties& props) const;
        void draw_text(const TextProperties& props);
        void draw_text(const std::string& str, Font* font, const Vector2f& position, float scale = 1.f, const Vector4f& color = { 1, 1, 1, 1 });

        inline const Shader& get_shader() const { return batch.get_shader(); }
        inline const Matrix4& get_proj_matrix() const { return batch.get_proj_matrix(); }
        inline const Matrix4& get_trans_matrix() const { return batch.get_trans_matrix(); }
        inline const Matrix4& get_combined_matrix() const { return batch.get_combined_matrix(); }

        inline void set_blending(bool blend = true){ batch.set_blending(blend); }
        inline void set_shader(Shader& shader){ batch.set_shader(shader); }
        inline void set_proj_matrix(const Matrix4& m){ batch.set_proj_matrix(m); }
        inline void set_trans_matrix(const Matrix4& m){ batch.set_trans_matrix(m); }

        void begin();
        void flush();
        void end();
    };
};