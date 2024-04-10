#define GLFW_INCLUDE_NONE

#include <string>
#include <iostream>

#include "draft/core/application.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/math/matrix.hpp"
#include "draft/util/logger.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"

namespace Draft {
    Application::Application(const char* title, const unsigned int width, const unsigned int height) : window(width, height, title) {
        // Feedback
        Logger::println(Level::INFO, "Draft Engine", "Initializing...");

        // Register things to load
        assetManager.queue_shader("./assets/shaders/test");
        assetManager.queue_texture("./assets/textures/test_image_1.png");
        assetManager.queue_texture("./assets/textures/test_image_3.png");
        assetManager.load();

        // Redirect cout to console
        oldOutBuf = std::cout.rdbuf(console.get_stream().rdbuf());

        // Register basic commands
        console.register_cmd("reload_assets", [this](ConsoleArgs args){
            assetManager.reload();

            Shader& testShader = assetManager.get_shader("./assets/shaders/test");
            testShader.bind();
            testShader.set_uniform("myTexture1", 0);
            testShader.set_uniform("myTexture2", 1);
            testShader.unbind();

            return true;
        });
        console.register_cmd("set_mouse", [this](ConsoleArgs args){
            Mouse::set_position({ stof(args[1]), stof(args[2]) });
            return true;
        });
    }

    Application::~Application(){
        // Cleanup
        Logger::println(Level::INFO, "Draft Engine", "Exitting...");

        // Restore cout to stdout
        std::cout.rdbuf(oldOutBuf);
    }

    void Application::run(){
        Matrix4 projection = Matrix4::perspective(45.f, 640/480.f, 0.1f, 100.f);
        Matrix4 transform = Matrix4::translation({ 0.5f, 0, -10.f }) * Matrix4::scale({ 10, 10, 10 });

        Shader& testShader = assetManager.get_shader("./assets/shaders/test");
        testShader.bind();
        testShader.set_uniform("myTexture1", 0);
        testShader.set_uniform("myTexture2", 1);
        testShader.unbind();

        Texture& testTexture1 = assetManager.get_texture("./assets/textures/test_image_1.png");
        Texture& testTexture2 = assetManager.get_texture("./assets/textures/test_image_3.png");

        VertexBuffer testBuffer{};
        testBuffer.buffer(0, {
            { 0.5f,  0.5f, 0.0f},  // top right
            { 0.5f, -0.5f, 0.0f},  // bottom right
            {-0.5f, -0.5f, 0.0f},  // bottom left
            {-0.5f,  0.5f, 0.0f}   // top left 
        });
        testBuffer.buffer(1, {
            {1.f, 0.f, 0.f},
            {0.f, 1.f, 0.f},
            {0.f, 0.f, 1.f},
            {1.f, 1.f, 0.f}
        });
        testBuffer.buffer(2, {
            {1.f, 1.f},
            {1.f, 0.f},
            {0.f, 0.f},
            {0.f, 1.f}
        });
        testBuffer.buffer(3, std::vector<int>{0, 1, 2, 0, 2, 3}, GL_ELEMENT_ARRAY_BUFFER);

        // Start application loop
        while(window.is_open()){
            // Clock reset
            deltaTime = deltaClock.restart();

            // Handle control events
            window.poll_events();

            // while(window.poll_event(event)){
            //     // ImGui::SFML::ProcessEvent(window.get_impl(), event);

            //     switch(event.type){
            //     case sf::Event::Closed:textureQueue
            //         window.close();
            //         break;
            //     default:
            //         // if(activeScene)
            //         //     activeScene->handleEvent(event);
            //         break;
            //     }
            // }

            // Handle updates and stuff
            if(activeScene)
                activeScene->update(deltaTime);

            window.clear();

            if(activeScene)
                activeScene->render(deltaTime);

            testShader.bind();
            testShader.set_uniform("testUniform", (float)glfwGetTime());
            testShader.set_uniform("model", transform * Matrix4::rotation({ (float)glfwGetTime(), (float)glfwGetTime() * 2, (float)glfwGetTime() * 2.5f }));
            testShader.set_uniform("projection", projection);
            testTexture1.bind(0);
            testTexture2.bind(1);
            testBuffer.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            testBuffer.unbind();

            console.draw();
            window.display();
        }
    }
}