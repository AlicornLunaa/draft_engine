#pragma once

#include "draft/interface/widgets/label.hpp"
#include "draft/interface/widgets/panel.hpp"

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
            // Variables
            bool disabled = false;

            // Constructors
            Button(SNumber x, SNumber y, SNumber w, SNumber h, bool* value, Type type = PRESS, Panel* parent = nullptr);

            // Events
            virtual bool handle_event(const Event& event) override;
            virtual void paint(Context& ctx) override;
        };

        class TextButton : public Button {
        public:
            // Variables
            Label label{0, 0, "", this};

            // Constructors
            TextButton(SNumber x, SNumber y, SNumber w, SNumber h, const std::string& text, bool* value, Type type = PRESS, Panel* parent = nullptr);
        };
    }
};