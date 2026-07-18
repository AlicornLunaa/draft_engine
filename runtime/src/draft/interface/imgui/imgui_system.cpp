#include "draft/interface/imgui/imgui_system.hpp"
#include "draft/input/keyboard.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <cfloat>

namespace {
    using namespace Draft;

    // Ported from imgui_impl_glfw's own ImGui_ImplGlfw_KeyToImGuiKey() switch. Keyboard::Key's
    // values are the raw GLFW keycodes themselves, so no GLFW include is needed here.
    ImGuiKey glfw_key_to_imgui_key(int keycode){
        switch(keycode){
            case Keyboard::TAB: return ImGuiKey_Tab;
            case Keyboard::LEFT: return ImGuiKey_LeftArrow;
            case Keyboard::RIGHT: return ImGuiKey_RightArrow;
            case Keyboard::UP: return ImGuiKey_UpArrow;
            case Keyboard::DOWN: return ImGuiKey_DownArrow;
            case Keyboard::PAGE_UP: return ImGuiKey_PageUp;
            case Keyboard::PAGE_DOWN: return ImGuiKey_PageDown;
            case Keyboard::HOME: return ImGuiKey_Home;
            case Keyboard::END: return ImGuiKey_End;
            case Keyboard::INSERT: return ImGuiKey_Insert;
            case Keyboard::DELETE: return ImGuiKey_Delete;
            case Keyboard::BACKSPACE: return ImGuiKey_Backspace;
            case Keyboard::SPACE: return ImGuiKey_Space;
            case Keyboard::ENTER: return ImGuiKey_Enter;
            case Keyboard::ESCAPE: return ImGuiKey_Escape;
            case Keyboard::APOSTROPHE: return ImGuiKey_Apostrophe;
            case Keyboard::COMMA: return ImGuiKey_Comma;
            case Keyboard::MINUS: return ImGuiKey_Minus;
            case Keyboard::PERIOD: return ImGuiKey_Period;
            case Keyboard::SLASH: return ImGuiKey_Slash;
            case Keyboard::SEMICOLON: return ImGuiKey_Semicolon;
            case Keyboard::EQUAL: return ImGuiKey_Equal;
            case Keyboard::LEFT_BRACKET: return ImGuiKey_LeftBracket;
            case Keyboard::BACKSLASH: return ImGuiKey_Backslash;
            case Keyboard::RIGHT_BRACKET: return ImGuiKey_RightBracket;
            case Keyboard::GRAVE: return ImGuiKey_GraveAccent;
            case Keyboard::CAPS_LOCK: return ImGuiKey_CapsLock;
            case Keyboard::SCROLL_LOCK: return ImGuiKey_ScrollLock;
            case Keyboard::NUM_LOCK: return ImGuiKey_NumLock;
            case Keyboard::PRINT_SCREEN: return ImGuiKey_PrintScreen;
            case Keyboard::PAUSE: return ImGuiKey_Pause;
            case Keyboard::KP0: return ImGuiKey_Keypad0;
            case Keyboard::KP1: return ImGuiKey_Keypad1;
            case Keyboard::KP2: return ImGuiKey_Keypad2;
            case Keyboard::KP3: return ImGuiKey_Keypad3;
            case Keyboard::KP4: return ImGuiKey_Keypad4;
            case Keyboard::KP5: return ImGuiKey_Keypad5;
            case Keyboard::KP6: return ImGuiKey_Keypad6;
            case Keyboard::KP7: return ImGuiKey_Keypad7;
            case Keyboard::KP8: return ImGuiKey_Keypad8;
            case Keyboard::KP9: return ImGuiKey_Keypad9;
            case Keyboard::KP_DECIMAL: return ImGuiKey_KeypadDecimal;
            case Keyboard::KP_DIVIDE: return ImGuiKey_KeypadDivide;
            case Keyboard::KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
            case Keyboard::KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
            case Keyboard::KP_ADD: return ImGuiKey_KeypadAdd;
            case Keyboard::KP_ENTER: return ImGuiKey_KeypadEnter;
            case Keyboard::KP_EQUAL: return ImGuiKey_KeypadEqual;
            case Keyboard::LEFT_SHIFT: return ImGuiKey_LeftShift;
            case Keyboard::LEFT_CONTROL: return ImGuiKey_LeftCtrl;
            case Keyboard::LEFT_ALT: return ImGuiKey_LeftAlt;
            case Keyboard::LEFT_SUPER: return ImGuiKey_LeftSuper;
            case Keyboard::RIGHT_SHIFT: return ImGuiKey_RightShift;
            case Keyboard::RIGHT_CONTROL: return ImGuiKey_RightCtrl;
            case Keyboard::RIGHT_ALT: return ImGuiKey_RightAlt;
            case Keyboard::RIGHT_SUPER: return ImGuiKey_RightSuper;
            case Keyboard::MENU: return ImGuiKey_Menu;
            case Keyboard::ZERO: return ImGuiKey_0;
            case Keyboard::ONE: return ImGuiKey_1;
            case Keyboard::TWO: return ImGuiKey_2;
            case Keyboard::THREE: return ImGuiKey_3;
            case Keyboard::FOUR: return ImGuiKey_4;
            case Keyboard::FIVE: return ImGuiKey_5;
            case Keyboard::SIX: return ImGuiKey_6;
            case Keyboard::SEVEN: return ImGuiKey_7;
            case Keyboard::EIGHT: return ImGuiKey_8;
            case Keyboard::NINE: return ImGuiKey_9;
            case Keyboard::A: return ImGuiKey_A;
            case Keyboard::B: return ImGuiKey_B;
            case Keyboard::C: return ImGuiKey_C;
            case Keyboard::D: return ImGuiKey_D;
            case Keyboard::E: return ImGuiKey_E;
            case Keyboard::F: return ImGuiKey_F;
            case Keyboard::G: return ImGuiKey_G;
            case Keyboard::H: return ImGuiKey_H;
            case Keyboard::I: return ImGuiKey_I;
            case Keyboard::J: return ImGuiKey_J;
            case Keyboard::K: return ImGuiKey_K;
            case Keyboard::L: return ImGuiKey_L;
            case Keyboard::M: return ImGuiKey_M;
            case Keyboard::N: return ImGuiKey_N;
            case Keyboard::O: return ImGuiKey_O;
            case Keyboard::P: return ImGuiKey_P;
            case Keyboard::Q: return ImGuiKey_Q;
            case Keyboard::R: return ImGuiKey_R;
            case Keyboard::S: return ImGuiKey_S;
            case Keyboard::T: return ImGuiKey_T;
            case Keyboard::U: return ImGuiKey_U;
            case Keyboard::V: return ImGuiKey_V;
            case Keyboard::W: return ImGuiKey_W;
            case Keyboard::X: return ImGuiKey_X;
            case Keyboard::Y: return ImGuiKey_Y;
            case Keyboard::Z: return ImGuiKey_Z;
            case Keyboard::F1: return ImGuiKey_F1;
            case Keyboard::F2: return ImGuiKey_F2;
            case Keyboard::F3: return ImGuiKey_F3;
            case Keyboard::F4: return ImGuiKey_F4;
            case Keyboard::F5: return ImGuiKey_F5;
            case Keyboard::F6: return ImGuiKey_F6;
            case Keyboard::F7: return ImGuiKey_F7;
            case Keyboard::F8: return ImGuiKey_F8;
            case Keyboard::F9: return ImGuiKey_F9;
            case Keyboard::F10: return ImGuiKey_F10;
            case Keyboard::F11: return ImGuiKey_F11;
            case Keyboard::F12: return ImGuiKey_F12;
            case Keyboard::F13: return ImGuiKey_F13;
            case Keyboard::F14: return ImGuiKey_F14;
            case Keyboard::F15: return ImGuiKey_F15;
            case Keyboard::F16: return ImGuiKey_F16;
            case Keyboard::F17: return ImGuiKey_F17;
            case Keyboard::F18: return ImGuiKey_F18;
            case Keyboard::F19: return ImGuiKey_F19;
            case Keyboard::F20: return ImGuiKey_F20;
            case Keyboard::F21: return ImGuiKey_F21;
            case Keyboard::F22: return ImGuiKey_F22;
            case Keyboard::F23: return ImGuiKey_F23;
            case Keyboard::F24: return ImGuiKey_F24;
            default: return ImGuiKey_None;
        }
    }
}

namespace Draft {
    // Constructors
    ImGuiSystem::ImGuiSystem(const Vector2u& size, const char* iniFilename, bool shouldBlockGameEvents)
        : m_size(size), shouldBlockGameEvents(shouldBlockGameEvents)
    {
        IMGUI_CHECKVERSION();

        ctx = ImGui::CreateContext();

        // CreateContext() restores whatever context was current before it ran (if any), it
        // doesn't leave the new one current. Every call below needs ctx current.
        ImGui::SetCurrentContext(ctx);

        auto& io = ImGui::GetIO();
        io.IniFilename = iniFilename;
        io.LogFilename = nullptr;
        io.BackendPlatformName = "draft";

        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 450");
    }

    ImGuiSystem::~ImGuiSystem(){
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext(ctx);
    }

    // Functions
    void ImGuiSystem::render(Time dt, RenderLayer layer){
        ImGui::SetCurrentContext(ctx);

        if(layer == RenderLayer::Default){
            auto& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)m_size.x, (float)m_size.y);
            io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
            io.DeltaTime = dt.as_seconds() > 0.f ? dt.as_seconds() : (1.f / 60.f);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
        } else if(layer == RenderLayer::Overlay){
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }

    bool ImGuiSystem::on_event(const Event& event){
        ImGui::SetCurrentContext(ctx);
        auto& io = ImGui::GetIO();

        switch(event.type){
            case Event::MouseMoved:
                io.AddMousePosEvent((float)event.mouseMove.x, (float)event.mouseMove.y);
                return io.WantCaptureMouse && shouldBlockGameEvents;

            case Event::MouseButtonPressed:
            case Event::MouseButtonReleased:
                io.AddMouseButtonEvent(event.mouseButton.button, event.type == Event::MouseButtonPressed);
                return io.WantCaptureMouse && shouldBlockGameEvents;

            case Event::MouseWheelScrolled:
                io.AddMouseWheelEvent((float)event.mouseWheelScroll.x, (float)event.mouseWheelScroll.y);
                return io.WantCaptureMouse && shouldBlockGameEvents;

            case Event::MouseLeft:
                io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
                return io.WantCaptureMouse && shouldBlockGameEvents;

            case Event::MouseEntered:
                // The next MouseMoved event covers this, nothing to do until then.
                return io.WantCaptureMouse && shouldBlockGameEvents;

            case Event::KeyPressed:
            case Event::KeyReleased:
            case Event::KeyHold: {
                io.AddKeyEvent(ImGuiMod_Ctrl, event.key.control);
                io.AddKeyEvent(ImGuiMod_Shift, event.key.shift);
                io.AddKeyEvent(ImGuiMod_Alt, event.key.alt);
                io.AddKeyEvent(ImGuiMod_Super, event.key.system);

                ImGuiKey key = glfw_key_to_imgui_key(event.key.code);
                io.AddKeyEvent(key, event.type != Event::KeyReleased);
                return io.WantCaptureKeyboard && shouldBlockGameEvents;
            }

            case Event::TextEntered:
                io.AddInputCharacter(event.text.unicode);
                return io.WantCaptureKeyboard && shouldBlockGameEvents;

            case Event::GainedFocus:
            case Event::LostFocus:
                io.AddFocusEvent(event.type == Event::GainedFocus);
                return false;

            case Event::Resized:
                resize({event.size.width, event.size.height});
                return false;

            default:
                return false;
        }
    }

    void ImGuiSystem::resize(const Vector2u& size){
        m_size = size;
    }

    bool ImGuiSystem::wants_keyboard_capture() const {
        ImGui::SetCurrentContext(ctx);
        return ImGui::GetIO().WantCaptureKeyboard;
    }

    bool ImGuiSystem::wants_mouse_capture() const {
        ImGui::SetCurrentContext(ctx);
        return ImGui::GetIO().WantCaptureMouse;
    }
}
