#define GLFW_INCLUDE_NONE

#include <string>

#include "draft/rendering/render_window.hpp"
#include "draft/input/event.hpp"
#include "draft/util/logger.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

namespace Draft {
    // Static variables
    std::unordered_map<void*, RenderWindow*> RenderWindow::glfwToRenderMap{};

    // Raw functions
    void window_size_callback(GLFWwindow* window, int width, int height){
        auto* renderWindow = RenderWindow::glfwToRenderMap[window];
        glViewport(0, 0, width, height);

        Event e{};
        e.type = Event::Resized;
        e.size.width = width;
        e.size.height = height;
        renderWindow->queue_event(e);
    }

    void window_focus_callback(GLFWwindow* window, int focused){
        auto* renderWindow = RenderWindow::glfwToRenderMap[window];
        Event e{};

        if(focused){
            // The window gained input focus
            e.type = Event::GainedFocus;
        } else {
            // The window lost input focus
            e.type = Event::LostFocus;
        }

        renderWindow->queue_event(e);
    }

    
    // Pimpl declaration
    struct RenderWindow::Impl {
        // Variables
        GLFWwindow* window = nullptr;

        // Constructors
        Impl(unsigned int w, unsigned int h, const string& title){
            // Callbacks
            glfwSetErrorCallback([](int errorCode, const char* errorDesc){
                Logger::println(Level::CRITICAL, "GLFW", string(errorDesc) + ", code: " + to_string(errorCode));
            });

            // Start GLFW
            if(glfwInit() == GLFW_FALSE)
                exit(0);

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
            glfwMakeContextCurrent(window);
            glfwSetWindowSizeCallback(window, window_size_callback);
            glfwSetWindowFocusCallback(window, window_focus_callback);
            glfwSwapInterval(1);

            // Start GLAD
            if(!gladLoadGL(glfwGetProcAddress)){
                Logger::println(Level::CRITICAL, "GLAD", "Failed to initialize");
                exit(0);
            }

            // Setup opengl context
            glViewport(0, 0, w, h);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Setup imgui
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            auto& imGuiIO = ImGui::GetIO();
            imGuiIO.IniFilename = nullptr;
            imGuiIO.LogFilename = nullptr;
            ImGui::StyleColorsDark();
            ImGui_ImplGlfw_InitForOpenGL(window, false);
            ImGui_ImplOpenGL3_Init("#version 450");
        }

        ~Impl(){
            // Cleanup ImGUI
            ImGui_ImplOpenGL3_Shutdown();
            ImGui:ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            // Cleanup opengl
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        // Functions
        void initialize_callbacks(){
            ImGui_ImplGlfw_InstallCallbacks(window);
        }
    };

    // Private functions
    void RenderWindow::init_callbacks(){
        ptr->initialize_callbacks();
    }

    // Definitions
    RenderWindow::RenderWindow(unsigned int width, unsigned int height, const string& title) : ptr(std::make_unique<Impl>(width, height, title)){
        // Save active window
        RenderWindow::glfwToRenderMap[ptr->window] = this;
    }
    RenderWindow::~RenderWindow(){
        // Remove active window
        RenderWindow::glfwToRenderMap[ptr->window] = nullptr;
    }

    // Functions
    void RenderWindow::queue_event(const Event& event){
        eventQueue.push(event);
    }

    const Vector2u RenderWindow::get_size() const {
        Vector2i size{};
        glfwGetWindowSize(ptr->window, &size.x, &size.y);
        return size;
    }

    void RenderWindow::set_size(const Vector2u& size){
        glfwSetWindowSize(ptr->window, size.x, size.y);
    }

    void RenderWindow::set_vsync(bool flag){ glfwSwapInterval(flag ? 1 : 0); }

    bool RenderWindow::is_open(){ return !glfwWindowShouldClose(ptr->window); }

    bool RenderWindow::poll_events(Event& event){
        glfwPollEvents();

        if(!eventQueue.empty()){
            event = eventQueue.front();
            eventQueue.pop();
            return true;
        }

        return false;
    }

    void RenderWindow::clear(){
        // Clear window
        glClearColor(0.05f, 0.05f, 0.05f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ImGUI frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void RenderWindow::display(){
        // Finalize ImGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Finalize frame
        glfwSwapBuffers(ptr->window);
    }

    void RenderWindow::close(){
        // Close window
        glfwSetWindowShouldClose(ptr->window, true);

        Event e{};
        e.type = Event::Closed;
        queue_event(e);
    }

    void* RenderWindow::get_raw_window(){
        return (void*)ptr->window;
    }
};