#include "draft/interface/imgui/imgui_system.hpp"
#include "draft/rendering/render_window.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Draft {
    // Constructors
    ImGuiSystem::ImGuiSystem(RenderWindow& window){
        IMGUI_CHECKVERSION();

        GLFWwindow* handle = window.get_glfw_handle();
        ctx = ImGui::CreateContext();

        // CreateContext() restores whatever context was current before it ran (if any), it
        // doesn't leave the new one current. Every call below needs ctx current.
        ImGui::SetCurrentContext(ctx);

        #ifndef DEBUG
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        #endif

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(handle, false);
        ImGui_ImplOpenGL3_Init("#version 450");
        ImGui_ImplGlfw_InstallCallbacks(handle);
    }

    ImGuiSystem::~ImGuiSystem(){
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(ctx);
    }

    // Functions
    void ImGuiSystem::render(Time, RenderLayer layer){
        ImGui::SetCurrentContext(ctx);

        if(layer == RenderLayer::Default){
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        } else if(layer == RenderLayer::Overlay){
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }

    bool ImGuiSystem::on_event(const Event& event){
        ImGui::SetCurrentContext(ctx);
        const ImGuiIO& io = ImGui::GetIO();

        switch(event.type){
            case Event::MouseMoved:
            case Event::MouseButtonPressed:
            case Event::MouseButtonReleased:
            case Event::MouseWheelScrolled:
            case Event::MouseEntered:
            case Event::MouseLeft:
                return io.WantCaptureMouse;

            case Event::KeyPressed:
            case Event::KeyReleased:
            case Event::KeyHold:
            case Event::TextEntered:
                return io.WantCaptureKeyboard;

            default:
                return false;
        }
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
