#pragma once

#include "draft/interface/widgets/panel.hpp"
#include "draft/rendering/batching/text_renderer.hpp"

#include <functional>
#include <string>
#include <vector>

namespace Draft {
    namespace UI {
        class TextInput : public Panel {
        private:
            // Types
            typedef std::function<void(std::string)> EnterFunc;

            // Variables
            TextRenderer textRenderer;
            std::string* str = nullptr;
            bool selected = false;
            EnterFunc enterFunc; // Function when enter is pressed

            std::vector<float> charWidth;
            uint stringStart = 0;
            uint stringLen = 0;
            uint cursorPos = 0;
            float animTimer = 0.f;

            // Private functions
            void cursor_left();
            void cursor_right();

        public:
            // Constructor
            TextInput(SNumber x, SNumber y, SNumber w, SNumber h, std::string* str, Panel* parent = nullptr);

            // Functions
            inline void set_enter_func(EnterFunc f){ enterFunc = f; }
            virtual bool handle_event(const Event& event) override;
            virtual void paint(Context& ctx) override;
        };
    };
};