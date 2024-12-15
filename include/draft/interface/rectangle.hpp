#pragma once

#include "draft/interface/context.hpp"
#include "draft/interface/panel.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    namespace UI {
        class Rectangle : public Panel {
        public:
            // Variables
            Vector4f color;

            // Constructor
            Rectangle(float x, float y, float w, float h, Panel* parent = nullptr);

            // Functions
            virtual void paint(Context& ctx) override;
        };
    };
};