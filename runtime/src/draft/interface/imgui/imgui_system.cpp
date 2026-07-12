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
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(ctx);
    }

    // Functions
    void ImGuiSystem::render(Time, RenderLayer layer){
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
}
