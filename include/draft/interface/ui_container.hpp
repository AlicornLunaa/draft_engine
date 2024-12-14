#pragma once

#include "draft/core/application.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/interface/panel.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    namespace UI {
        struct Style {
            // Stylesheet
            Vector4f backgroundColor = { 1, 1, 1, 1 };
            Vector4f textColor = { 0, 0, 0, 1 };
        };

        class Container {
        private:
            // Variables
            const Application* app = nullptr;
            OrthographicCamera camera;
            FloatRect windowBounds;

            Resource<Shader> uiShader;
            TextRenderer textRenderer;

        public:
            // Constructors
            Container(const Application* app, const Vector2f& size, Resource<Shader> uiShader = Assets::manager.get<Shader>("assets/shaders/default", true));
            Container(const Container& other) = delete;
            ~Container() = default;

            // Functions
            inline OrthographicCamera& get_camera(){ return camera; }

            template<typename T>
            void draw(){
                
            }

            void remove_panel(Panel* panel);

            bool handle_event(const Event& event);
            void render(const Time& deltaTime, SpriteBatch& batch);
        };
    };
};