#pragma once

#include "draft/rendering/vertex_buffer.hpp"
#include "draft/math/glm.hpp"

#include <vector>

namespace Draft {
    class Panel;

    class UIContainer {
    public:
        // Structs
        struct Vertex {
            Vector2f position{0, 0};
            Vector2f texCoord{0, 0};
            Vector4f color{1, 1, 1, 1};
        };

    private:
        // Variables
        std::vector<Panel> panels;
        size_t dynamicBufferLocation = 0;
        size_t currentBufferSize = 0; // In vertices
        size_t currentIndicesCount = 0;
        VertexBuffer* buffer = nullptr;

        // Private functions
        void resize_buffer(size_t vertexCount);
        void check_buffer_can_store();
        void validate_panels();

    public:
        // Constructors
        UIContainer();
        UIContainer(const UIContainer& other) = delete;
        ~UIContainer();

        // Functions
        void add_panel(const Panel& panel);
        void render();
    };
};