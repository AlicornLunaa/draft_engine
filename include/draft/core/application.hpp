#pragma once

#include "draft/core/scene.hpp"
#include "draft/input/event.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/asset_manager.hpp"
#include "draft/util/clock.hpp"
#include "draft/widgets/console.hpp"
#include "draft/widgets/stats.hpp"

namespace Draft {
    /**
     * @brief The main application. Renders the scene pointer supplied, if any.
     * Contains a console and delta time. Redirects iostream to the window's
     * console. Fixed timestep supplied.
     */
    class Application {
    private:
        std::streambuf* oldOutBuf = nullptr;
        Scene* activeScene = nullptr;
        Clock deltaClock;
        Event event;
        double accumulator = 0.0; // Used for fixed timestep

    public:
        AssetManager assets;
        RenderWindow window;
        Keyboard keyboard;
        Time deltaTime;
        double timeStep = 1.0/66.0;

        Console console{this};
        Stats stats;
        bool debug = true;
        
        Application(const char* title, const unsigned int width, const unsigned int height);
        Application(const Application& rhs) = delete; // Dont allow copying.
        ~Application();

        /**
         * @brief Runs the application. This starts and spawns the main application loop.
         */
        void run();

        inline void set_scene(Scene* scene){ activeScene = scene; }
        inline Scene* get_scene(){ return activeScene; }
    };
}