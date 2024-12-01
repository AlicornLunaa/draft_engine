#include "draft/interface/text_input.hpp"
#include "draft/input/event.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/rendering/batching/text_renderer.hpp"

namespace Draft {
    // Constructor
    TextInput::TextInput(float x, float y, float w, float h, std::string* str, Panel* parent) : Panel(parent), str(str) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
    }

    // Functions
    bool TextInput::handle_event(const Event& event){
        switch(event.type){
        case Event::MouseButtonPressed:
            selected = Math::contains(bounds, {event.mouseButton.x, event.mouseButton.y});
            return selected;

        case Event::KeyPressed:
            if(event.key.code == Keyboard::BACKSPACE){ str->erase(str->begin() + str->size() - 1); }
            // if(event.key.code == Keyboard::DELETE){ str->erase(str->begin() + str->size()); }
            break;

        case Event::TextEntered:
            if(selected){
                *str += (char)event.text.unicode;
                return true;
            }
            
            return false;

        default:
            break;
        }

        return false;
    }
    
    void TextInput::paint(const Time& deltaTime, SpriteBatch& batch){
        // Get info for rendering correctly
        TextProperties props{
            *str,
            nullptr,
            {bounds.x + 10, bounds.y + bounds.height * 0.5f},
            {0, 0.5f},
            {1, 1, 1, 1},
            0.f,
            1.f
        };
        textBounds = textRenderer.get_text_bounds(props);

        // Basic rectangle which changes color based on its value
        if(selected){
            // Cursor
            animTimer += deltaTime.as_seconds() * 2.f;

            if((int)animTimer % 2 == 0){
                batch.draw({
                    nullptr,
                    {},
                    {bounds.x + textBounds.x + 4, bounds.y + bounds.height * 0.5f},
                    0.f,
                    {6, bounds.height * 0.75f},
                    {0, bounds.height * 0.75f * 0.5f},
                    0.f,
                    {0.8, 0.8, 0.8, 1},
                    false
                });
            }
        } else {
            animTimer = 0.f;
        }

        batch.draw({
            nullptr,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            0.f,
            {0.3, 0.3, 0.3, 1},
            false
        });

        textRenderer.draw_text(batch, props);
    }
};