#pragma once

#include "draft/interface/widgets/panel.hpp"
#include <string>

namespace Draft {
    namespace UI {
        class Label : public Panel {
        public:
            // Variables
            TextProperties properties;

            // Constructors
            Label(SNumber x, SNumber y, std::string text, Panel* parent = nullptr);

            // Events
            virtual void paint(Context& ctx) override;
        };
    }
};