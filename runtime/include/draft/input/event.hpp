#pragma once

#include <cstdint>
#include <functional>

namespace Draft {
    /**
     * @brief A single input/window event, tagged by `type` with the matching member of the union
     * valid to read. Backend-agnostic on purpose, nothing here names GLFW. A window layer
     * translates its own callbacks
     */
    class Event {
    public:
        struct SizeEvent {
            unsigned int width;
            unsigned int height;
        };

        struct KeyEvent {
            int code;
            bool alt;
            bool control;
            bool shift;
            bool system;
            int mods;
        };

        struct TextEvent {
            uint32_t unicode;
        };

        struct MouseMoveEvent {
            int x;
            int y;
        };

        struct MouseButtonEvent {
            int button;
            int x;
            int y;
            int mods;
        };

        struct MouseWheelScrollEvent {
            double x;
            double y;
        };

        enum EventType {
            Closed,
            Resized,
            LostFocus,
            GainedFocus,
            TextEntered,
            KeyPressed,
            KeyReleased,
            KeyHold,
            MouseWheelScrolled,
            MouseButtonPressed,
            MouseButtonReleased,
            MouseMoved,
            MouseEntered,
            MouseLeft,

            Count
        };

        EventType type;

        union {
            SizeEvent size;
            KeyEvent key;
            TextEvent text;
            MouseMoveEvent mouseMove;
            MouseButtonEvent mouseButton;
            MouseWheelScrollEvent mouseWheelScroll;
        };
    };

    /**
     * @brief True return means "consumed, stop propagating this event further" (see
     * SystemRegistry::dispatch_event()/Scene::dispatch_event()/Application::eventCallback).
     */
    typedef std::function<bool(const Event& event)> EventCallback;
}
