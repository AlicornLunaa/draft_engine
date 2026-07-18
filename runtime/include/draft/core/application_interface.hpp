#pragma once

#include "draft/ecs/scene.hpp"
#include "draft/input/event.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/rendering/pipeline/renderer.hpp"
#include "draft/rendering/render_target.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/clock.hpp"

#include <memory>

namespace Draft {
    /**
     * @brief Shared surface between `Application` (owns the real window, drives the real
     * run()/step() loop straight to the backbuffer) and `SubApplication` (binds to an
     * already-existing window/keyboard/mouse, renders into an offscreen Framebuffer instead of a
     * real backbuffer).
     */
    class ApplicationInterface {
    public:
        // Public variables
        RenderWindow& window;
        Keyboard& keyboard;
        Mouse& mouse;

        Time deltaTime;
        Time timeStep = Time::seconds(1.0 / 66.0);
        Time maxAccumulator = Time::seconds(1.0 / 5.0);

        /**
         * @brief When true, tick() stops calling the active Scene's update() (and resets its
         * accumulator so no burst of steps fires once unpaused). Rendering is unaffected. Meant
         * for freezing gameplay while still showing the scene, e.g. an editor in edit mode, or a
         * game's own pause menu.
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
        ApplicationInterface(RenderWindow& window, Keyboard& keyboard, Mouse& mouse);
        ApplicationInterface(const ApplicationInterface& other) = delete;
        ApplicationInterface& operator=(const ApplicationInterface& other) = delete;
        virtual ~ApplicationInterface() = default;

        // Functions
        void set_scene(Scene* scene);
        inline Scene* get_scene() const { return p_activeScene; }

        void set_renderer(std::unique_ptr<Renderer> renderer);
        inline Renderer* get_renderer() const { return p_renderer.get(); }

    protected:
        // Protected functions
        bool dispatch(const Event& event);

        void scene_change();
        void reset_timers();

        /**
         * @brief Fixed-timestep Scene::update(), gated by simulationPaused.
         */
        void tick();

        /**
         * @brief Runs one frame's worth of Scene::render()/Renderer::render_frame() into
         * @p target, beginning/ending it around the work.
         */
        void frame_into(RenderTarget& target);

        // Protected variables
        std::unique_ptr<Renderer> p_renderer;
        std::unique_ptr<Renderer> p_newRenderer = nullptr;

        Scene* p_activeScene = nullptr;
        Scene* p_newScene = nullptr;

        Clock p_deltaClock;
        double p_accumulator = 0.0;
    };
}
