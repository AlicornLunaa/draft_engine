#define GLFW_INCLUDE_NONE

#include "draft/interface/rmlui/rml_backend.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/logger.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include "tracy/TracyOpenGL.hpp"

#include <memory>
#include <string>

using namespace std;

namespace Draft {
    // Constructors
    RenderWindow::RenderWindow(unsigned int width, unsigned int height, const string& title) : Window(width, height, title) {
        // Start GLAD
        if(!gladLoadGL(glfwGetProcAddress)){
            Logger::println(Level::CRITICAL, "GLAD", "Failed to initialize");
            exit(0);
        }

        // Setup opengl context
        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Start gpu profiler
        TracyGpuContext;

        // Setup imgui
        IMGUI_CHECKVERSION();

        ImGui::CreateContext();
        auto& imGuiIO = ImGui::GetIO();
        imGuiIO.IniFilename = nullptr;
        imGuiIO.LogFilename = nullptr;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(get_glfw_handle(), false);
        ImGui_ImplOpenGL3_Init("#version 450");
        ImGui_ImplGlfw_InstallCallbacks(get_glfw_handle());

        // Save active window for RMLUI
        m_rmlBackend = std::make_unique<UI::RMLBackend>(*this);
    }

    RenderWindow::~RenderWindow(){
        // Cleanup ImGUI
        ImGui_ImplOpenGL3_Shutdown();
        ImGui:ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // Functions
    void RenderWindow::clear(const Vector4f& clearColor){
        // Profiler frame start
        TracyGpuZone("window_clear");

        // Clear window
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ImGUI frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void RenderWindow::display(){
        // End gpu frame
        TracyGpuZone("window_display");

        // Finalize ImGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Finalize frame
        swap_buffers();

        // Collect profiler data
        TracyGpuCollect;
    }
};