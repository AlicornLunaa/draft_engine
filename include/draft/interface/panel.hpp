#pragma once

#include "draft/input/event.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/util/time.hpp"

#include <vector>

namespace Draft {
    class Panel {
    private:
        // Variables
        std::vector<Panel*> children;
        Panel* parent = nullptr;

    protected:
        // Variables to be modified by children
        FloatRect bounds;

        // Protected functions
        const Panel* get_parent() const { return parent; }

    public:
        // Constructors
        Panel(Panel* parent = nullptr);
        virtual ~Panel() = default;

        // Friends
        friend class UIContainer;

        // Functions
        virtual bool handle_event(const Event& event){ return false; };
        virtual void paint(const Time& deltaTime, SpriteBatch& batch);
        const FloatRect& get_bounds() const { return bounds; }
    };
};