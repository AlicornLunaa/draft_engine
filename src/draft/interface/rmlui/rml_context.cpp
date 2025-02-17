#include "draft/interface/rmlui/rml_context.hpp"
#include "RmlUi/Core/Types.h"
#include "draft/input/keyboard.hpp"
#include "draft/math/glm.hpp"

#include "RmlUi/Core/Input.h"
#include "RmlUi/Core/Core.h"

#include <cassert>

namespace Draft {
    // Private functions
    int RmlContext::get_modifiers(int draft_modifiers){
        int state = 0;
        state |= ((draft_modifiers & static_cast<int>(Keyboard::Modifier::ALT)) ? Rml::Input::KM_ALT : 0);
        state |= ((draft_modifiers & static_cast<int>(Keyboard::Modifier::CTRL)) ? Rml::Input::KM_CTRL : 0);
        state |= ((draft_modifiers & static_cast<int>(Keyboard::Modifier::SHIFT)) ? Rml::Input::KM_SHIFT : 0);
        state |= ((draft_modifiers & static_cast<int>(Keyboard::Modifier::SUPER)) ? Rml::Input::KM_META : 0);
        state |= ((draft_modifiers & static_cast<int>(Keyboard::Modifier::CAPS_LOCK)) ? Rml::Input::KM_CAPSLOCK : 0);
        state |= ((draft_modifiers & static_cast<int>(Keyboard::Modifier::NUM_LOCK)) ? Rml::Input::KM_NUMLOCK : 0);
        return state;
    }

    Rml::Input::KeyIdentifier RmlContext::get_key(int draftKey){
        switch(draftKey){
            case Keyboard::SPACE: return Rml::Input::KeyIdentifier::KI_SPACE;
            case Keyboard::COMMA: return Rml::Input::KeyIdentifier::KI_OEM_COMMA;
            case Keyboard::MINUS: return Rml::Input::KeyIdentifier::KI_OEM_MINUS;
            case Keyboard::PERIOD: return Rml::Input::KeyIdentifier::KI_OEM_PERIOD;
            case Keyboard::ZERO: return Rml::Input::KeyIdentifier::KI_0;
            case Keyboard::ONE: return Rml::Input::KeyIdentifier::KI_1;
            case Keyboard::TWO: return Rml::Input::KeyIdentifier::KI_2;
            case Keyboard::THREE: return Rml::Input::KeyIdentifier::KI_3;
            case Keyboard::FOUR: return Rml::Input::KeyIdentifier::KI_4;
            case Keyboard::FIVE: return Rml::Input::KeyIdentifier::KI_5;
            case Keyboard::SIX: return Rml::Input::KeyIdentifier::KI_6;
            case Keyboard::SEVEN: return Rml::Input::KeyIdentifier::KI_7;
            case Keyboard::EIGHT: return Rml::Input::KeyIdentifier::KI_8;
            case Keyboard::NINE: return Rml::Input::KeyIdentifier::KI_9;
            case Keyboard::EQUAL: return Rml::Input::KeyIdentifier::KI_OEM_NEC_EQUAL;
            case Keyboard::A: return Rml::Input::KeyIdentifier::KI_A;
            case Keyboard::B: return Rml::Input::KeyIdentifier::KI_B;
            case Keyboard::C: return Rml::Input::KeyIdentifier::KI_C;
            case Keyboard::D: return Rml::Input::KeyIdentifier::KI_D;
            case Keyboard::E: return Rml::Input::KeyIdentifier::KI_E;
            case Keyboard::F: return Rml::Input::KeyIdentifier::KI_F;
            case Keyboard::G: return Rml::Input::KeyIdentifier::KI_G;
            case Keyboard::H: return Rml::Input::KeyIdentifier::KI_H;
            case Keyboard::I: return Rml::Input::KeyIdentifier::KI_I;
            case Keyboard::J: return Rml::Input::KeyIdentifier::KI_J;
            case Keyboard::K: return Rml::Input::KeyIdentifier::KI_K;
            case Keyboard::L: return Rml::Input::KeyIdentifier::KI_L;
            case Keyboard::M: return Rml::Input::KeyIdentifier::KI_M;
            case Keyboard::N: return Rml::Input::KeyIdentifier::KI_N;
            case Keyboard::O: return Rml::Input::KeyIdentifier::KI_O;
            case Keyboard::P: return Rml::Input::KeyIdentifier::KI_P;
            case Keyboard::Q: return Rml::Input::KeyIdentifier::KI_Q;
            case Keyboard::R: return Rml::Input::KeyIdentifier::KI_R;
            case Keyboard::S: return Rml::Input::KeyIdentifier::KI_S;
            case Keyboard::T: return Rml::Input::KeyIdentifier::KI_T;
            case Keyboard::U: return Rml::Input::KeyIdentifier::KI_U;
            case Keyboard::V: return Rml::Input::KeyIdentifier::KI_V;
            case Keyboard::W: return Rml::Input::KeyIdentifier::KI_W;
            case Keyboard::X: return Rml::Input::KeyIdentifier::KI_X;
            case Keyboard::Y: return Rml::Input::KeyIdentifier::KI_Y;
            case Keyboard::Z: return Rml::Input::KeyIdentifier::KI_Z;
            case Keyboard::ESCAPE: return Rml::Input::KeyIdentifier::KI_ESCAPE;
            case Keyboard::TAB: return Rml::Input::KeyIdentifier::KI_TAB;
            case Keyboard::BACKSPACE: return Rml::Input::KeyIdentifier::KI_BACK;
            case Keyboard::INSERT: return Rml::Input::KeyIdentifier::KI_INSERT;
            case Keyboard::DELETE: return Rml::Input::KeyIdentifier::KI_DELETE;
            case Keyboard::RIGHT: return Rml::Input::KeyIdentifier::KI_RIGHT;
            case Keyboard::LEFT: return Rml::Input::KeyIdentifier::KI_LEFT;
            case Keyboard::DOWN: return Rml::Input::KeyIdentifier::KI_DOWN;
            case Keyboard::UP: return Rml::Input::KeyIdentifier::KI_UP;
            case Keyboard::HOME: return Rml::Input::KeyIdentifier::KI_HOME;
            case Keyboard::END: return Rml::Input::KeyIdentifier::KI_END;
            case Keyboard::CAPS_LOCK: return Rml::Input::KeyIdentifier::KI_CAPITAL;
            case Keyboard::SCROLL_LOCK: return Rml::Input::KeyIdentifier::KI_SCROLL;
            case Keyboard::NUM_LOCK: return Rml::Input::KeyIdentifier::KI_NUMLOCK;
            case Keyboard::PRINT_SCREEN: return Rml::Input::KeyIdentifier::KI_PRINT;
            case Keyboard::PAUSE: return Rml::Input::KeyIdentifier::KI_PAUSE;
            case Keyboard::F1: return Rml::Input::KeyIdentifier::KI_F1;
            case Keyboard::F2: return Rml::Input::KeyIdentifier::KI_F2;
            case Keyboard::F3: return Rml::Input::KeyIdentifier::KI_F3;
            case Keyboard::F4: return Rml::Input::KeyIdentifier::KI_F4;
            case Keyboard::F5: return Rml::Input::KeyIdentifier::KI_F5;
            case Keyboard::F6: return Rml::Input::KeyIdentifier::KI_F6;
            case Keyboard::F7: return Rml::Input::KeyIdentifier::KI_F7;
            case Keyboard::F8: return Rml::Input::KeyIdentifier::KI_F8;
            case Keyboard::F9: return Rml::Input::KeyIdentifier::KI_F9;
            case Keyboard::F10: return Rml::Input::KeyIdentifier::KI_F10;
            case Keyboard::F11: return Rml::Input::KeyIdentifier::KI_F11;
            case Keyboard::F12: return Rml::Input::KeyIdentifier::KI_F12;
            case Keyboard::F13: return Rml::Input::KeyIdentifier::KI_F13;
            case Keyboard::F14: return Rml::Input::KeyIdentifier::KI_F14;
            case Keyboard::F15: return Rml::Input::KeyIdentifier::KI_F15;
            case Keyboard::F16: return Rml::Input::KeyIdentifier::KI_F16;
            case Keyboard::KP_DECIMAL: return Rml::Input::KeyIdentifier::KI_DECIMAL;
            case Keyboard::KP_DIVIDE: return Rml::Input::KeyIdentifier::KI_DIVIDE;
            case Keyboard::KP_MULTIPLY: return Rml::Input::KeyIdentifier::KI_MULTIPLY;
            case Keyboard::KP_SUBTRACT: return Rml::Input::KeyIdentifier::KI_SUBTRACT;
            case Keyboard::KP_ADD: return Rml::Input::KeyIdentifier::KI_ADD;
            case Keyboard::KP_EQUAL: return Rml::Input::KeyIdentifier::KI_OEM_NEC_EQUAL;
            default: return Rml::Input::KI_UNKNOWN;
        }
    }

    // Constructors
    RmlContext::RmlContext(RmlEngine& engine, const std::string& name, const Vector2i& size){
        // Save the engine for later rendering
        m_engine = &engine;

        // Create context
        m_context = Rml::CreateContext(name, {size.x, size.y});
    }

    RmlContext::RmlContext(RmlContext&& other) : m_engine(other.m_engine), m_context(other.m_context) {
        other.m_engine = nullptr;
        other.m_context = nullptr;
    }

    RmlContext::~RmlContext(){
        Rml::RemoveContext(m_context->GetName());
    }

    // Operators
    RmlContext& RmlContext::operator=(RmlContext&& other){
        if(this != &other){
            m_engine = other.m_engine;
            m_context = other.m_context;
            other.m_engine = nullptr;
            other.m_context = nullptr;
        }

        return *this;
    }

    // Functions
    Rml::ElementDocument* RmlContext::create_document(const std::string& name) const {
        Rml::ElementDocument* document = m_context->CreateDocument(name);
        return document;
    }

    Rml::ElementDocument* RmlContext::load_document(const std::string& path) const {
        Rml::ElementDocument* document = m_context->LoadDocument(path);
        return document;
    }

    void RmlContext::handle_event(const Event& event){
        // Dispatch events to the context
        switch(event.type){
            case Event::MouseButtonPressed:
                m_context->ProcessMouseButtonDown(event.mouseButton.button, get_modifiers(event.mouseButton.mods));
                break;

            case Event::MouseButtonReleased:
                m_context->ProcessMouseButtonUp(event.mouseButton.button, get_modifiers(event.mouseButton.mods));
                break;

            case Event::MouseMoved:
                m_context->ProcessMouseMove(event.mouseMove.x, event.mouseMove.y, 0);
                break;

            case Event::MouseWheelScrolled:
                m_context->ProcessMouseWheel({(float)event.mouseWheelScroll.x, (float)event.mouseWheelScroll.y}, 0);
                break;

            case Event::MouseLeft:
                m_context->ProcessMouseLeave();
                break;

            case Event::KeyPressed:
                m_context->ProcessKeyDown(get_key(event.key.code), get_modifiers(event.key.mods));
                break;

            case Event::KeyReleased:
                m_context->ProcessKeyUp(get_key(event.key.code), get_modifiers(event.key.mods));
                break;

            case Event::TextEntered:
                m_context->ProcessTextInput(Rml::Character(event.key.code));
                
            default:
                break;
        }
    }

    void RmlContext::render() const {
        // Render this context
        assert(is_valid() && "Trying to render an invalid context is prohibited");
    
        m_engine->start_frame();
        m_context->Update();
        m_context->Render();
        m_engine->end_frame();
    }

    bool RmlContext::is_valid() const {
        return m_context != nullptr && m_engine != nullptr;
    }
}