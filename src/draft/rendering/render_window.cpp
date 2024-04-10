#include "draft/input/event.hpp"
#define GLFW_INCLUDE_NONE

#include <format>

#include "draft/rendering/render_window.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/util/logger.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

namespace Draft {
    // Pimpl declaration
    struct RenderWindow::Impl {
        // Variables
        GLFWwindow* window = nullptr;

        // Constructors
        Impl(unsigned int w, unsigned int h, const string& title){
            // Callbacks
            glfwSetErrorCallback([](int errorCode, const char* errorDesc){
                Logger::println(Level::CRITICAL, "GLFW", format("{}, code: {}", errorDesc, errorCode));
            });

            // Start GLFW
            if(glfwInit() == GLFW_FALSE)
                exit(0);

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
            glfwMakeContextCurrent(window);

            // Start GLAD
            if(!gladLoadGL(glfwGetProcAddress)){
                Logger::println(Level::CRITICAL, "GLAD", "Failed to initialize");
                exit(0);
            }

            // Setup opengl context
            glViewport(0, 0, w, h);
            glEnable(GL_DEPTH_TEST);

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

    // Definitions
    RenderWindow::RenderWindow(unsigned int width, unsigned int height, const string& title) : ptr(std::make_unique<Impl>(width, height, title)) {
        // Setup inputs
        Keyboard::init(this);
        Mouse::init(this);

        Keyboard::add_callback([this](Event e){ eventQueue.emplace(e); });
        Mouse::add_callback([this](Event e){ eventQueue.emplace(e); });

        ptr->initialize_callbacks();
    }
    RenderWindow::~RenderWindow(){
        Mouse::clear_callbacks();
        Keyboard::clear_callbacks();
    }

    // Functions
    const Vector2u RenderWindow::get_size(){
        Vector2i size{};
        glfwGetWindowSize(ptr->window, &size.x, &size.y);
        return size;
    }

    bool RenderWindow::is_open(){ return !glfwWindowShouldClose(ptr->window); }

    bool RenderWindow::poll_events(Event& event){
        glfwPollEvents();

        event = eventQueue.front();
        eventQueue.pop();
        return !eventQueue.empty();
    }

    void RenderWindow::clear(){
        // Clear window
        glClearColor(0.2, 0.3, 0.4, 1.f);
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
    }

    void* RenderWindow::get_raw_window(){
        return (void*)ptr->window;
    }
};