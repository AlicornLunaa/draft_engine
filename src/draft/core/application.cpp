#define GLFW_INCLUDE_NONE

#include <string>
#include <iostream>

#include "draft/core/application.hpp"
#include "draft/core/scene.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/logger.hpp"
#include "draft/widgets/stats.hpp"
#include "glad/gl.h"

namespace Draft {
    Application::Application(const char* title, const unsigned int width, const unsigned int height) : window(width, height, title) {
        // Feedback
        Logger::println(Level::INFO, "Draft Engine", "Initializing...");

        // Register things to load
        assetManager.queue_shader("./assets/shaders/default");
        assetManager.queue_shader("./assets/shaders/shapes");
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
    }

    Application::~Application(){
        // Cleanup
        Logger::println(Level::INFO, "Draft Engine", "Exitting...");

        // Restore cout to stdout
        std::cout.rdbuf(oldOutBuf);
    }

    void Application::run(){
        VertexBuffer testBuffer{};
        testBuffer.buffer(0, {
            { 0.5f,  0.5f, 0.0f},  // top right
            { 0.5f, -0.5f, 0.0f},  // bottom right
            {-0.5f, -0.5f, 0.0f},  // bottom left
            {-0.5f,  0.5f, 0.0f}   // top left 
        });
        testBuffer.buffer(1, {
            {1.f, 1.f},
            {1.f, 0.f},
            {0.f, 0.f},
            {0.f, 1.f}
        });
        testBuffer.buffer(2, std::vector<int>{0, 1, 2, 0, 2, 3}, GL_ELEMENT_ARRAY_BUFFER);

        VertexBuffer cubeBuffer{};
        cubeBuffer.start_buffer({
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        });
        cubeBuffer.set_attribute(0, 3, sizeof(float) * 5, 0);
        cubeBuffer.set_attribute(1, 2, sizeof(float) * 5, sizeof(float) * 3);
        cubeBuffer.end_buffer();

        // Start application loop
        while(window.is_open()){
            // Clock reset
            deltaTime = deltaClock.restart();

            // Handle control events
            while(window.poll_events(event)){
                switch(event.type){
                case Event::Closed:
                    window.close();
                    break;
                default:
                    if(activeScene)
                        activeScene->handleEvent(event);
                    break;
                }
            }

            // Fixed time-step
            accumulator += deltaTime.as_seconds();

            while(accumulator >= timeStep){
                if(activeScene){
                    activeScene->update(deltaTime);
                }

                accumulator -= timeStep;
            }
            
            // Rendering stuff!
            window.clear();

            if(activeScene)
                activeScene->render(deltaTime);

            // testShader.bind();
            // testShader.set_uniform("testUniform", (float)glfwGetTime());
            // testShader.set_uniform("model", transform);
            // testShader.set_uniform("view", camera.get_view());
            // testShader.set_uniform("projection", camera.get_projection());
            // testTexture1.bind(0);
            // testTexture2.bind(1);

            // testShader.set_uniform("model", transform * Matrix4::translation({ 1.1f, 0, 0 }));
            // testBuffer.bind();
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            // testBuffer.unbind();
            
            // testShader.set_uniform("model", transform);
            // cubeBuffer.bind();
            // glDrawArrays(GL_TRIANGLES, 0, 36);
            // cubeBuffer.unbind();

            // Draw debug stuff
            if(debug){
                Stats::draw(*this);
            }

            console.draw();
            window.display();
        }
    }
}