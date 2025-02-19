#pragma once

namespace Draft {
    /// Macros for easy safety
    #define ATTACH_LISTENER(element, event, listener) { \
        if(element){                                    \
            element->AddEventListener(event, listener); \
        }                                               \
    }

    /// Forward decls
    class Event;
    class RmlContext;

    /// Interface class for layouts. This should manage event listener lifetimes in conjunction with
    /// context lifetimes.
    class IRmlLayout {
    public:
        virtual bool handle_event(const Event& event) = 0;
        virtual void render() = 0;
        virtual RmlContext* get_ctx() { return nullptr; };
    };
}