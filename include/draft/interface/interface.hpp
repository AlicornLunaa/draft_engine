#pragma once

#include "draft/core/scene.hpp"
#include "draft/interface/dom_tree.hpp"
#include "draft/interface/context.hpp"
#include "draft/interface/widgets/layout.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/batching/shape_batch.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/clip.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/util/asset_manager/resource.hpp"

#include <cmath>

namespace Draft {
    namespace UI {
        // Main control panel for rendering
        class Interface {
        private:
            // Variables
            Scene* scene = nullptr;
            SpriteBatch batch;
            ShapeBatch shapes;
            TextRenderer textBatch;
            Resource<Shader> defaultShader = Assets::manager.get<Shader>("assets/shaders/default");
            Clip scissor;

            OrthoCamera camera = {{0, 0, 10}, {0, 0, -1}, 0, 1280, 720, 0};
            Context masterCtx;

            void concrete_phase(DomTree* parent, DomTree* node, const Vector2f& winSize);

        public:
            // Constructors
            Interface(Scene* scene);
            ~Interface() = default;

            // Functions
            void begin();
            void draw(const Layout& layout);
            void end();
        };
    };
};