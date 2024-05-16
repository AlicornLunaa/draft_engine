#include "draft/input/event.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/shader.hpp"
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
        
        for(auto* panel : panels){
            totalVertices += panel->vertices.size();
        }

        if(totalVertices > currentBufferSize){
            // Too many vertices to store, resize it
            resize_buffer(totalVertices);

            // Ensures all the data is stored at correct locations, even if vertices increase in an existing container
            for(auto* panel : panels){
                panel->invalidate();
            }
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

            currentPanelOffset += (panel->vertices.size() * sizeof(Vertex));
        }
    }

    // Constructors
    UIContainer::UIContainer(const Application* app, const Vector2f& size, const std::shared_ptr<Shader> uiShader) : buffer(new VertexBuffer()), windowBounds({0, 0, size.x, size.y}),
        uiCamera({{ 0, 0, 10 }, { 0, 0, -1 }, 0, size.x, 0, size.y, 0.1f, 100.f}), uiShader(uiShader), app(app) {}

    UIContainer::~UIContainer(){
        if(buffer)
            delete buffer;

        for(auto* p : panels)
            delete p;

        panels.clear();
    }

    // Functions
    bool UIContainer::handle_event(const Event& event){
        // Update event positions for modern stuff
        Vector2f vec;

        Event eventCpy(event);
        switch(event.type){
        case Event::MouseButtonPressed:
        case Event::MouseButtonReleased:
            vec = uiCamera.unproject(Math::normalize_coordinates(windowBounds, {event.mouseButton.x, event.mouseButton.y}));
            eventCpy.mouseButton.x = vec.x;
            eventCpy.mouseButton.y = vec.y;
            break;

        case Event::MouseMoved:
            vec = uiCamera.unproject(Math::normalize_coordinates(windowBounds, {event.mouseMove.x, event.mouseMove.y}));
            eventCpy.mouseMove.x = vec.x;
            eventCpy.mouseMove.y = vec.y;
            break;

        default:
            break;
        }

        // Handles events for each panel
        for(auto* p : panels){
            if(p->handle_event(eventCpy))
                return true;
        }

        return false;
    }

    void UIContainer::render(const Time& deltaTime){
        // Check for window resize
        auto size = app->window.get_size();
        windowBounds.width = size.x;
        windowBounds.height = size.y;

        // Handle updates
        for(auto* p : panels)
            p->update(deltaTime);

        // Make sure every panel is up to date
        validate_panels();

        // Render everything
        uiShader->bind();
        uiCamera.apply(app->window, uiShader);
        buffer->bind();
        glDrawArrays(GL_TRIANGLES, 0, currentBufferSize);
        buffer->unbind();
    }
};