#pragma once

#include "draft/core/application.hpp"
#include "draft/interface/context.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/interface/widgets/panel.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/time.hpp"

namespace Draft {
    namespace UI {
        class Container {
        private:
            // Variables
            const Application* app = nullptr;
            OrthographicCamera camera{{ 0, 0, 10 }, { 0, 0, -1 }, 0, 1, 0, 1, 0.1f, 100.f};
            FloatRect windowBounds{{0, 0}, app->window.get_size()};

            std::vector<std::unique_ptr<Panel>> panels;
            Resource<Shader> uiShader;
            TextRenderer textRenderer;

        public:
            // Variables
            Style stylesheet; // Global stylesheet to use

            // Constructors
            Container(const Application* app, Resource<Shader> uiShader = Assets::manager.get<Shader>("assets/shaders/default", true));
            Container(const Container& other) = delete;
            ~Container() = default;

            // Functions
            inline OrthographicCamera& get_camera(){ return camera; }

            template<typename T>
            T* add_panel(T* panel){
                panels.push_back(std::unique_ptr<Panel>(panel));
                return panel;
            }

            void remove_panel(Panel* panel);

            bool handle_event(const Event& event);
            void render(const Time& deltaTime, SpriteBatch& batch);
        };
    };
};