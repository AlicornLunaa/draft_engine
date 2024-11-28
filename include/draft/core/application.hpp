#pragma once

#include "draft/core/scene.hpp"
#include "draft/input/event.hpp"
#include "draft/input/mouse.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/clock.hpp"
#include "draft/interface/widgets/console.hpp"
#include "draft/interface/widgets/stats.hpp"

namespace Draft {
    /**
     * @brief The main application. Renders the scene pointer supplied, if any.
     * Contains a console and delta time. Redirects iostream to the window's
     * console. Fixed timestep supplied.
     */
    class Application {
    private:
        // Private vars
        Scene* activeScene = nullptr;
        Clock deltaClock;
        Event event;
        double accumulator = 0.0; // Used for fixed timestep

        // Private functions
        void handle_events();
        void tick();
        void frame();

    public:
        RenderWindow window;
        Mouse mouse;
        Keyboard keyboard;
        Time deltaTime;
        Time timeStep = Time::seconds(1.0/66.0);
        Time maxAccumulator = Time::seconds(1.0/5.0);

        Console console{this};
        Stats stats;
        bool debug = true;
        
        Application(const char* title, const unsigned int width, const unsigned int height);
        Application(const Application& rhs) = delete; // Dont allow copying.
        Application& operator=(const Application& rhs) = delete;
        ~Application();

        /**
         * @brief Runs the application. This starts and spawns the main application loop.
         */
        void run();

        void reset_timers();

        void set_scene(Scene* scene);
        Scene* get_scene() const;
    };
}