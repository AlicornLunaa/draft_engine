#pragma once

#include "draft/core/scene.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/asset_manager.hpp"
#include "draft/widgets/console.hpp"

namespace Draft {
    class Application {
    private:
        std::streambuf* oldOutBuf = nullptr;
        Scene* activeScene = nullptr;
        // sf::Clock deltaClock;
        // sf::Event event;

    public:
        AssetManager assetManager;
        RenderWindow window;
        Console console;
        // sf::Time deltaTime;
        
        Application(const char* title, const unsigned int width, const unsigned int height);
        Application(const Application& rhs) = delete;
        ~Application();

        void run();

        // void setScene(Scene* scene){ activeScene = scene; }
        // Scene* getScene(){ return activeScene; }
    };
}