#include "draft/rendering/vertex_buffer.hpp"
#include "draft/interface/ui_container.hpp"
#include "draft/interface/panel.hpp"

#include "glad/gl.h"
#include <cstddef>
#include <vector>

namespace Draft {
    // Private functions
    void UIContainer::resize_buffer(size_t vertexCount){
        // Constructs a new buffer and resizes it to the bytes selected
        if(buffer) delete buffer;
        buffer = new VertexBuffer();
        currentBufferSize = vertexCount;

        dynamicBufferLocation = buffer->start_buffer<Vertex>(vertexCount);
        buffer->set_attribute(0, GL_FLOAT, 2, sizeof(Vertex), 0);
        buffer->set_attribute(1, GL_FLOAT, 2, sizeof(Vertex), offsetof(Vertex, texCoord));
        buffer->set_attribute(2, GL_FLOAT, 4, sizeof(Vertex), offsetof(Vertex, color));
        buffer->end_buffer();
    }

    void UIContainer::check_buffer_can_store(){
        // Check sizes
        size_t totalVertices = 0;
        
        for(auto& panel : panels){
            totalVertices += panel->size();
        }

        if(totalVertices > currentBufferSize){
            // Too many vertices to store, resize it
            resize_buffer(totalVertices);
        }
    }

    void UIContainer::validate_panels(){
        // Check for invalid panels
        check_buffer_can_store();
        size_t currentPanelOffset = 0; // Offset for current location in buffer

        for(size_t i = 0; i < panels.size(); i++){
            auto& panel = panels[i];

            if(!panel->validLayout){
                // This panel is not a validated layout rebuffer dynamic data
                buffer->set_dynamic_data(dynamicBufferLocation, panel->vertices, currentPanelOffset);
                panel->validLayout = true;
            }

            currentPanelOffset += (panel->size() * sizeof(Vertex));
        }
    }

    // Constructors
    UIContainer::UIContainer() : buffer(new VertexBuffer()) {}
    UIContainer::~UIContainer(){
        if(buffer)
            delete buffer;

        for(auto* p : panels)
            delete p;

        panels.clear();
    }

    // Functions
    bool UIContainer::handle_event(const Event& event){
        // Handles events for each panel
        for(auto* p : panels){
            if(p->handle_event(event))
                return true;
        }

        return false;
    }

    void UIContainer::render(){
        // Make sure every panel is up to date
        validate_panels();

        // Render everything
        buffer->bind();
        glDrawArrays(GL_TRIANGLES, 0, currentBufferSize);
        buffer->unbind();
    }
};