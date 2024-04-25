#pragma once

#include "draft/interface/ui_container.hpp"

#include <vector>

namespace Draft {
    class Panel {
    private:
        // Variables
        bool validLayout = false;

    protected:
        // Variables to be modified by children
        std::vector<UIContainer::Vertex> vertices;

    public:
        // Constructors
        Panel(size_t vertexCount);

        // Friends
        friend class UIContainer;

        // Functions
        inline void invalidate(){ validLayout = false; }
        inline size_t size(){ return vertices.size(); }
    };
};