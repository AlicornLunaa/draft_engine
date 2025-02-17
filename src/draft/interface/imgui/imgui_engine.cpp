#include "draft/interface/imgui/imgui_engine.hpp"
#include "draft/rendering/render_window.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Draft {
    // Constructors
    ImguiEngine::ImguiEngine(RenderWindow& window){
        // Initialize ImGui
        IMGUI_CHECKVERSION();

        GLFWwindow* d_window = window.get_glfw_handle();
        ctx = ImGui::CreateContext();

        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(d_window, false);
        ImGui_ImplOpenGL3_Init("#version 450");
        ImGui_ImplGlfw_InstallCallbacks(d_window);
    }

    ImguiEngine::ImguiEngine(ImguiEngine&& other) : ctx(other.ctx) {
        other.ctx = nullptr;
    }

    ImguiEngine::~ImguiEngine(){
        // Destroy ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui:ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // Operators
    ImguiEngine& ImguiEngine::operator=(ImguiEngine&& other){
        if(this != &other){
            ctx = other.ctx;
            other.ctx = nullptr;
        }

        return *this;
    }

    // Functions
    void ImguiEngine::start_frame() const {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImguiEngine::end_frame() const {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}