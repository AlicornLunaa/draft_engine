#define GLFW_INCLUDE_NONE
#include "draft/core/application.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/logger.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"

namespace Draft {
    Application::Application(const char* title, const unsigned int width, const unsigned int height) : window(width, height, title) {
        // Feedback
        Logger::println(Level::INFO, "Draft Engine", "Initializing...");
        
        // Make sure game can run, handle arguments, etc
        // if(!sf::Shader::isAvailable()){
        //     Logger::println(Level::CRITICAL, "Draft Engine", "Shaders unavailable, maybe OpenGL is too old? Cannot continue.");
        //     exit(1);
        // }

        // ImGUI
        // if(!ImGui::SFML::Init(window.get_impl())){
        //     Logger::println(Level::CRITICAL, "ImGUI", "Failed to initialize, exitting.");
        //     exit(1);
        // }
        // auto& imGuiIO = ImGui::GetIO();
        // imGuiIO.IniFilename = nullptr;
        // imGuiIO.LogFilename = nullptr;

        // Register basic commands
        // console.registerCmd("reload_assets", [this](ConsoleArgs args){
        //     assetManager.reload();
        //     return true;
        // });

        // Redirect cout to console
        // oldOutBuf = std::cout.rdbuf(console.getStream().rdbuf());
    }

    Application::~Application(){
        // Cleanup
        Logger::println(Level::INFO, "Draft Engine", "Exitting...");

        // Restore cout to stdout
        // std::cout.rdbuf(oldOutBuf);
    }

    void Application::run(){
        Shader testShader("./assets/shaders/test");

        VertexBuffer testBuffer{};
        testBuffer.buffer(0, {
            { 0.5f,  0.5f, 0.0f},  // top right
            { 0.5f, -0.5f, 0.0f},  // bottom right
            {-0.5f, -0.5f, 0.0f},  // bottom left
            {-0.5f,  0.5f, 0.0f}   // top left 
        });
        testBuffer.buffer(1, std::vector<int>{0, 1, 2, 0, 2, 3}, GL_ELEMENT_ARRAY_BUFFER);

        // Start application loop
        while(window.is_open()){
            // Clock reset
            // deltaTime = deltaClock.restart();

            // Handle control events
            window.poll_events();
            window.render();

            testShader.bind();
            testShader.set_uniform("test_uniform", (float)glfwGetTime());
            testBuffer.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            testBuffer.unbind();

            window.swap_buffers();
            // while(window.poll_event(event)){
            //     // ImGui::SFML::ProcessEvent(window.get_impl(), event);

            //     switch(event.type){
            //     case sf::Event::Closed:
            //         window.close();
            //         break;
            //     default:
            //         // if(activeScene)
            //         //     activeScene->handleEvent(event);
            //         break;
            //     }
            // }

            // Handle updates and stuff
            // ImGui::SFML::Update(window.get_impl(), deltaTime);
            // if(activeScene)
            //     activeScene->update(deltaTime);

            // Handle SFML rendering
            // window.clear();

            // if(activeScene)
            //     activeScene->render(deltaTime);

            // window.display();
        }
    }
}