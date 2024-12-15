#include "draft/interface/panel.hpp"
#include "draft/math/bounds.hpp"
#include "draft/math/glm.hpp"

namespace Draft::UI {
    // Constructor
    Panel::Panel(Panel* parent) : parent(parent) {
        // Add this panel to the parent's children so we can calculate down the tree too
        if(parent) parent->children.push_back(this);
    }

    // Functions
    bool Panel::handle_event(const Event& event){
        // Skip with no children to save cpu time
        if(children.empty())
            return false;

        // Convert event to panel-relative coordinates
        Event eventCpy(event);
        bool mouseEvent = false;

        switch(event.type){
        case Event::MouseButtonPressed:
        case Event::MouseButtonReleased:
            eventCpy.mouseButton.x = event.mouseButton.x - bounds.x;
            eventCpy.mouseButton.y = event.mouseButton.y - bounds.y;

            // Skip if not for this element
            if(!Math::contains(bounds, {event.mouseButton.x, event.mouseButton.y}))
                return false;

            break;

        case Event::MouseMoved:
            eventCpy.mouseMove.x = event.mouseMove.x - bounds.x;
            eventCpy.mouseMove.y = event.mouseMove.y - bounds.y;

            // Skip if not for this element
            if(!Math::contains(bounds, {event.mouseMove.x, event.mouseMove.y}))
                return false;
            
            break;

        default:
            break;
        }

        // Handle events of children
        for(auto* ptr : children){
            if(ptr->handle_event(eventCpy)){
                return true;
            }
        }

        return false;
    }

    void Panel::paint(Context& ctx){
        // Update real position with fake relative pos
        if(position.x < 0){
            bounds.x = ctx.parentSize.x - size.x + position.x;
        } else {
            bounds.x = position.x;
        }

        if(position.y < 0){
            bounds.y = -position.y;
        } else {
            bounds.y = ctx.parentSize.y - size.y - position.y;
        }

        bounds.width = size.x;
        bounds.height = size.y;

        // Handle painting of children
        if(children.empty())
            return;

        // Create scissor clip, first need the window coordinates of this panel
        Vector2f halfWindowSize = static_cast<Vector2f>(ctx.windowSize) * 0.5f;
        Vector2f panelWindowPos = static_cast<Vector2f>(ctx.projection * Vector4f(bounds.x, bounds.y, 0, 1)) * halfWindowSize + halfWindowSize;
        scissor.box.x = panelWindowPos.x;
        scissor.box.y = panelWindowPos.y;
        scissor.box.width = bounds.width;
        scissor.box.height = bounds.height;

        if(parent){
            scissor.box.x += parent->bounds.x;
            scissor.box.y += parent->bounds.y;
        }

        // Update context for sizes and update state with new real position
        Vector2f previousCtxSize = ctx.parentSize;
        ctx.parentSize = {bounds.width, bounds.height};

        // Update state so it renders local to this panel
        Batch& batch = ctx.batch;
        Matrix4 previousTransform = ctx.batch.get_trans_matrix();
        batch.set_trans_matrix(Math::translate(previousTransform, {bounds.x, bounds.y, 0}));
        scissor.begin();

        for(auto* ptr : children){
            ptr->layer = layer + 0.1f;
            ptr->paint(ctx);
        }

        batch.set_trans_matrix(previousTransform);
        scissor.end(); // This works because set_trans_matrix flushes the batch

        // Reset state
        ctx.parentSize = previousCtxSize;
    }

    void Panel::add_child(Panel* ptr){
        if(ptr->parent)
            ptr->parent->remove_child(ptr);
        
        ptr->parent = this;
        children.push_back(ptr);
    }

    void Panel::remove_child(Panel* ptr){
        ptr->parent = nullptr;
        
        for(uint i = 0; i < children.size(); i++){
            if(children[i] == ptr){
                children.erase(children.begin() + i);
                break;
            }
        }
    }
};