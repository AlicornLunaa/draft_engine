#pragma once

#include "draft/rendering/vertex_buffer.hpp"
#include "draft/interface/panel.hpp"

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
        template<typename T>
        T* add_panel(T* panel){
            panels.push_back(panel);
            return panel;
        }

        void render();
    };
};