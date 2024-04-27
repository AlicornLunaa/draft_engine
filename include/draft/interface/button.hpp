#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    namespace UI {
        class Button : public Panel {
        public:
            enum Type { PRESS, TOGGLE };

        private:
            FloatRect bounds;
            Type type;
            bool* value;

            void update_btn();

        public:
            Button(float x, float y, float w, float h, bool* value, Type type = Button::Type::PRESS, Panel* parent = nullptr);

            virtual bool handle_event(const Event& event);
        };
    }
};