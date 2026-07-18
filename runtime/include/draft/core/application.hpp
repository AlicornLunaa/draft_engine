#pragma once

#include "draft/ecs/scene.hpp"
#include "draft/input/event.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/clock.hpp"

#include <memory>
#include <string>

namespace Draft {
    /**
     * @brief Owns the window/keyboard/mouse, a swappable Renderer, and the fixed-timestep main
     * loop. Holds and drives a Scene
     */
    class Application {
    public:
        // Public variables
        RenderWindow window;
        Keyboard keyboard;
        Mouse mouse;

        Time deltaTime;
        Time timeStep = Time::seconds(1.0 / 66.0);
        Time maxAccumulator = Time::seconds(1.0 / 5.0);

        /**
         * @brief When true, tick() stops calling the active Scene's update() (and resets its
         * accumulator so no burst of steps fires once unpaused). Rendering is unaffected, frame()
         * always runs. Meant for freezing gameplay while still showing the scene, e.g. an editor
         * in edit mode, or a game's own pause menu.
         */
        bool simulationPaused = false;

        /**
         * @brief Called first for every translated Event, before the active Scene's systems see
         * it, for embedder-level concerns not tied to any particular Scene (e.g. an editor's
         * own global shortcuts). Returning true consumes the event, the same as a system's
         * on_event() returning true, the active Scene's systems never see it.
         */
        EventCallback eventCallback = nullptr;

        // Constructors
        Application(const std::string& title, unsigned int width, unsigned int height);
        Application(const Application& other) = delete;
        Application& operator=(const Application& other) = delete;
        ~Application() = default;

        // Functions
        /**
         * @brief Runs step() in a loop until the window closes.
         */
        void run();

        /**
         * @brief Runs exactly one iteration of the main loop
         * @return Whether the window is still open (mirrors window.is_open()).
         */
        bool step();

        void set_scene(Scene* scene);
        inline Scene* get_scene() const { return p_activeScene; }

        void set_renderer(std::unique_ptr<Renderer> renderer);
        inline Renderer* get_renderer() const { return p_renderer.get(); }

    private:
        // Private functions
        bool dispatch(const Event& event);

        void framebuffer_resized(unsigned int width, unsigned int height);
        void window_focus(bool focused);
        void window_closed();
        void key_callback(int key, int action, int modifier);
        void text_callback(unsigned int codepoint);
        void mouse_button_callback(int button, int action, int modifier);
        void mouse_position_callback(const Vector2d& position);
        void mouse_scroll_callback(const Vector2d& delta);
        void mouse_enter_callback();
        void mouse_leave_callback();

        void trigger_resize(unsigned int width, unsigned int height);
        void scene_change();
        void reset_timers();

        void tick();
        void frame();

        // Variables
        std::unique_ptr<Renderer> p_renderer;
        std::unique_ptr<Renderer> p_newRenderer = nullptr;

        Scene* p_activeScene = nullptr;
        Scene* p_newScene = nullptr;

        Clock p_deltaClock;
        double p_accumulator = 0.0;
        bool p_pendingResize = false;
    };
}
