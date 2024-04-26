#pragma once

#include "draft/input/event.hpp"
#include "draft/interface/ui_vertex.hpp"
#include "draft/math/rect.hpp"
#include "draft/util/time.hpp"

#include <vector>

namespace Draft {
    class Panel {
    private:
        // Variables
        Panel* parent = nullptr;
        bool validLayout = false;

    protected:
        // Variables to be modified by children
        std::vector<Vertex> vertices;
        FloatRect bounds;

        // Protected functions
        const Panel* get_parent() const { return parent; }

    public:
        // Constructors
        Panel(size_t vertexCount, Panel* parent = nullptr);

        // Friends
        friend class UIContainer;

        // Functions
        virtual bool handle_event(const Event& event){ return false; };
        virtual void update(const Time& deltaTime){}
        inline void invalidate(){ validLayout = false; }
        const FloatRect& get_bounds() const { return bounds; }
    };
};