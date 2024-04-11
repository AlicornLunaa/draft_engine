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

        double accumulator = 0.0;
        double timeStep = 1.0/66.0;

    public:
        AssetManager assetManager;
        RenderWindow window;
        Console console;
        Time deltaTime;
        bool debug = true;
        
        Application(const char* title, const unsigned int width, const unsigned int height);
        Application(const Application& rhs) = delete;
        ~Application();

        void run();

        inline void set_time_step(double v){ timeStep = v; }
        inline double get_time_step(){ return timeStep; }

        inline void set_scene(Scene* scene){ activeScene = scene; }
        inline Scene* get_scene(){ return activeScene; }
    };
}