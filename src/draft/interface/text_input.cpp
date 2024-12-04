#include "draft/interface/text_input.hpp"
#include "draft/input/event.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/rendering/batching/text_renderer.hpp"

namespace Draft {
    // Private functions
    void TextInput::cursor_left(){
        // Moves the cursor left
        cursorPos--;

        if(cursorPos <= stringStart && stringStart > 0){
            stringStart--;
        }
    }

    void TextInput::cursor_right(){
        // Moves the cursor right
        cursorPos++;

        if(cursorPos > stringStart + stringLen && stringStart < stringLen){
            stringStart++;
        }
    }

    // Constructor
    TextInput::TextInput(float x, float y, float w, float h, std::string* str, Panel* parent) : Panel(parent), str(str) {
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;

        *str = "";
    }

    // Functions
    bool TextInput::handle_event(const Event& event){
        switch(event.type){
        case Event::MouseButtonReleased:
            selected = Math::contains(bounds, {event.mouseButton.x, event.mouseButton.y});
            return selected;

        case Event::KeyHold:
        case Event::KeyPressed:
            // Handle all text controls
            if(selected && str->length() > 0){
                if(event.key.code == Keyboard::BACKSPACE && cursorPos > 0){
                    // Backspace removes the character behind the cursor
                    str->erase(str->begin() + cursorPos - 1);
                    charWidth.erase(charWidth.begin() + cursorPos);
                    cursor_left();
                } else if(event.key.code == Keyboard::DELETE && cursorPos < str->length()){
                    // Delete removes the character in front of the cursor
                    str->erase(str->begin() + cursorPos);
                    charWidth.erase(charWidth.begin() + cursorPos);
                } else if(event.key.code == Keyboard::LEFT && cursorPos > 0){
                    // Shifts the cursor left
                    cursor_left();
                } else if(event.key.code == Keyboard::RIGHT && cursorPos < str->length()){
                    // Shifts the cursor right
                    cursor_right();
                } else if(event.key.code == Keyboard::HOME){
                    // Set cursor to the beginning
                    cursorPos = 0;
                    stringStart = 0;
                } else if(event.key.code == Keyboard::END){
                    // Sets cursor to the end
                    cursorPos = str->length();
                    stringStart = std::max((uint)0, (uint)str->length() - stringLen);
                } else if(event.key.code == Keyboard::ESCAPE){
                    // Unselect the bar
                    selected = false;
                } else if(event.key.code == Keyboard::ENTER){
                    // Sets cursor to the end
                    selected = false;

                    if(enterFunc)
                        enterFunc(*str);
                }
            }
            break;

        case Event::TextEntered:
            // Handle text inputs
            if(selected){
                // Use this to get the width of the string
                TextProperties props{
                    std::string(1, (char)event.text.unicode),
                    nullptr,
                    {bounds.x + 10, bounds.y + bounds.height * 0.5f},
                    {0, 0.5f},
                    {1, 1, 1, 1},
                    0.f,
                    1.f
                };

                (*str).insert(cursorPos, props.str);
                charWidth.insert(charWidth.begin() + cursorPos, textRenderer.get_text_bounds(props).x);
                stringLen = Math::min((float)str->length(), bounds.width / 16.f - 1);
                cursor_right();

                return true;
            }
            break;

        default:
            break;
        }

        return false;
    }
    
    void TextInput::paint(const Time& deltaTime, SpriteBatch& batch){
        // Get info for rendering correctly. Render from the start of the string
        TextProperties props{
            (*str).substr(stringStart, stringLen),
            nullptr,
            {bounds.x + 10, bounds.y + bounds.height * 0.5f},
            {0, 0.5f},
            {1, 1, 1, 1},
            0.f,
            1.f
        };

        // Basic rectangle which changes color based on its value
        if(selected){
            // Cursor
            animTimer += deltaTime.as_seconds() * 2.f;

            if((int)animTimer % 2 == 0){
                float cursorPixelPos = 0.f;

                for(uint c = stringStart; c < cursorPos; c++)
                    cursorPixelPos += charWidth[c];

                batch.draw({
                    nullptr,
                    {},
                    {bounds.x + cursorPixelPos + 10, bounds.y + bounds.height * 0.5f},
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