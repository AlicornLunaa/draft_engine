#pragma once

#include "draft/interface/widgets/panel.hpp"
#include "draft/interface/context.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    namespace UI {
        class Rectangle : public Panel {
        public:
            // Variables
            Vector4f color{1, 1, 1, 1};
            
            // Constructor
            Rectangle(SNumber x, SNumber y, SNumber w, SNumber h, Panel* parent = nullptr);

            // Functions
            virtual void paint(Context& ctx) override;
        };
    };
};