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

#include <memory>
#include <vector>

namespace Draft {
    class UIContainer {
    private:
        // Variables
        const Application* app = nullptr;
        OrthographicCamera camera;
        FloatRect windowBounds;

        std::vector<std::unique_ptr<Panel>> panels;
        Resource<Shader> uiShader;
        TextRenderer textRenderer;

    public:
        // Constructors
        UIContainer(const Application* app, const Vector2f& size, Resource<Shader> uiShader = Assets::manager.get<Shader>("assets/shaders/default", true));
        UIContainer(const UIContainer& other) = delete;
        ~UIContainer() = default;

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