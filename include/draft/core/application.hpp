#pragma once

#include "draft/core/scene.hpp"
#include "draft/input/event.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/asset_manager.hpp"
#include "draft/util/clock.hpp"
#include "draft/widgets/console.hpp"

namespace Draft {
    class Application {
    private:
        std::streambuf* oldOutBuf = nullptr;
        Scene* activeScene = nullptr;
        Clock deltaClock;
        Event event;

    public:
        AssetManager assetManager;
        RenderWindow window;
        Console console;
        Time deltaTime;
        
        Application(const char* title, const unsigned int width, const unsigned int height);
        Application(const Application& rhs) = delete;
        ~Application();

        void run();

        void set_scene(Scene* scene){ activeScene = scene; }
        Scene* get_scene(){ return activeScene; }
    };
}