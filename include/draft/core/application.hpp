#pragma once

#include "draft/core/scene.hpp"
#include "draft/input/event.hpp"
#include "draft/input/mouse.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/interface/rmlui/rml_engine.hpp"
#include "draft/interface/imgui/console.hpp"
#include "draft/interface/imgui/stats.hpp"
#include "draft/util/clock.hpp"

namespace Draft {
    /**
     * @brief The main application. Renders the scene pointer supplied, if any.
     * Contains a console and delta time. Redirects iostream to the window's
     * console. Fixed timestep supplied.
     */
    class Application {
    private:
        // Private vars
        Scene* m_activeScene = nullptr;
        Clock m_deltaClock;
        Event m_event;
        double m_accumulator = 0.0; // Used for fixed timestep

        // Private functions
        void framebuffer_resized(uint width, uint height);
        void window_focus(bool focused);
        void window_closed();
        void key_callback(int key, int action, int modifier);
        void text_callback(unsigned int codepoint);
        void mouse_button_callback(int button, int action, int modifier);
        void mouse_position_callback(const Vector2d& position);
        void mouse_scroll_callback(const Vector2d& delta);
        void mouse_enter_callback();
        void mouse_leave_callback();

        void tick();
        void frame();

    public:
        RenderWindow window;
        Keyboard keyboard;
        Mouse mouse;
        Time deltaTime;
        Time timeStep = Time::seconds(1.0/66.0);
        Time maxAccumulator = Time::seconds(1.0/5.0);

        ImguiEngine imgui{window}; // Created here because it will override keyboard and mouse callbacks
        RmlEngine rml{window};

        Console console{this};
        Stats stats;
        bool debug = true;
        
        Application(const char* title, const unsigned int width, const unsigned int height);
        Application(const Application& rhs) = delete; // Dont allow copying.
        Application& operator=(const Application& rhs) = delete;
        virtual ~Application();

        /**
         * @brief Runs the application. This starts and spawns the main application loop.
         */
        void run();

        void reset_timers();

        void set_scene(Scene* scene);
        Scene* get_scene() const;
    };
}