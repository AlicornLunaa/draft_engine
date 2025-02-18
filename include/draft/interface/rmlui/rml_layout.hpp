#pragma once

namespace Draft {
    /// Forward decls
    class Event;
    class RmlContext;

    /// Interface class for layouts. This should manage event listener lifetimes in conjunction with
    /// context lifetimes.
    class IRmlLayout {
    public:
        virtual bool handle_event(const Event& event) = 0;
        virtual void render() const = 0;
        virtual RmlContext* get_ctx() const { return nullptr; };
    };
}