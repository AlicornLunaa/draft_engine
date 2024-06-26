#pragma once

#include "draft/core/application.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/interface/panel.hpp"
#include "draft/util/asset_manager.hpp"

#include <vector>

namespace Draft {
    class UIContainer {
    private:
        // Variables
        std::vector<Panel*> panels;
        size_t dynamicBufferLocation = 0;
        size_t currentBufferSize = 0; // In vertices
        size_t currentIndicesCount = 0;
        VertexBuffer* buffer = nullptr;

        const Application* app = nullptr;
        const Shader& uiShader;
        OrthographicCamera uiCamera;
        FloatRect windowBounds;

        // Private functions
        void resize_buffer(size_t vertexCount);
        void check_buffer_can_store();
        void validate_panels();

    public:
        // Constructors
        UIContainer(const Application* app, const Vector2f& size, const Shader& uiShader = Assets::get_asset<Shader>("assets/shaders/interface"));
        UIContainer(const UIContainer& other) = delete;
        ~UIContainer();

        // Functions
        template<typename T>
        T* add_panel(T* panel){
            panels.push_back(panel);
            return panel;
        }

        bool handle_event(const Event& event);
        void render(const Time& deltaTime);
    };
};