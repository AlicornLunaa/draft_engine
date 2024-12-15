#include "draft/input/event.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/interface/ui_container.hpp"
#include "draft/interface/panel.hpp"

#include "glad/gl.h"
#include <cstddef>
#include <vector>

namespace Draft::UI {
    // Constructors
    Container::Container(const Application* app, const Vector2f& size, Resource<Shader> uiShader) : uiShader(uiShader), windowBounds({0, 0, size.x, size.y}),
        camera({{ 0, 0, 10 }, { 0, 0, -1 }, 0, size.x, 0, size.y, 0.1f, 100.f}), app(app) {}

    // Functions
    void Container::remove_panel(Panel* panel){
        // Remove panel and and its children
        for(Panel* p : panel->children){
            remove_panel(p);
        }

        // Linear search, shouldn't matter too much because element count is smallish
        for(size_t i = 0; i < panels.size(); i++){
            if(panels[i].get() == panel){
                panels.erase(panels.begin() + i);
                break;
            }
        }
    }

    bool Container::handle_event(const Event& event){
        // Update event positions for modern stuff
        Vector2f vec;

        Event eventCpy(event);
        switch(event.type){
        case Event::MouseButtonPressed:
        case Event::MouseButtonReleased:
            vec = camera.unproject(Math::normalize_coordinates(windowBounds, {event.mouseButton.x, event.mouseButton.y}));
            eventCpy.mouseButton.x = vec.x;
            eventCpy.mouseButton.y = vec.y;
            break;

        case Event::MouseMoved:
            vec = camera.unproject(Math::normalize_coordinates(windowBounds, {event.mouseMove.x, event.mouseMove.y}));
            eventCpy.mouseMove.x = vec.x;
            eventCpy.mouseMove.y = vec.y;
            break;

        default:
            break;
        }

        // Handles events for each top-level panel, they'll pass events to their children if needed
        for(auto& p : panels){
            if(!p->parent && p->handle_event(eventCpy)){
                return true;
            }
        }

        return false;
    }

    void Container::render(const Time& deltaTime, SpriteBatch& batch){
        // Check for window resize
        auto size = app->window.get_size();
        windowBounds.width = size.x;
        windowBounds.height = size.y;

        // Update camera
        camera = OrthographicCamera(
            camera.get_position(),
            camera.get_forward(),
            0,
            (float)size.x,
            0,
            (float)size.y,
            camera.get_near(),
            camera.get_far()
        );

        // Render everything
        batch.set_proj_matrix(camera.get_combined());
        batch.set_trans_matrix(Matrix4(1.f));
        batch.set_shader(uiShader);

        Context ctx{camera.get_combined(), size, deltaTime, batch, textRenderer, size};

        for(int i = (panels.size() - 1); i >= 0; i--){
            // Paint top-level widgets. They'll handle rendering their children
            if(!panels[i]->parent) panels[i]->paint(ctx);
        }
    }
};