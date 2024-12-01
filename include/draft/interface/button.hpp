#pragma once

#include "draft/interface/panel.hpp"

namespace Draft {
    namespace UI {
        class Button : public Panel {
        public:
            // Press is a push button and toggle is like a switch
            enum Type { PRESS, TOGGLE };

        private:
            // Store the button type and pointer to where its state is being stored/modified
            Type type;
            bool* value;

        public:
            // Constructors
            Button(float x, float y, float w, float h, bool* value, Type type = PRESS, Panel* parent = nullptr);

            // Events
            virtual bool handle_event(const Event& event) override;
            virtual void paint(const Time& deltaTime, SpriteBatch& batch) override;
        };
    }
};