#pragma once

#include "draft/interface/widgets/rectangle.hpp"
#include "draft/interface/widgets/panel.hpp"
#include "draft/interface/context.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    namespace UI {
        class ScrollPane : public Panel {
        private:
            // Variables
            Rectangle handle;
            Vector2f mousePosition{};
            bool grabbing = false; // Used for click & drag

            std::vector<std::unique_ptr<Panel>> items;

        public:
            // Public variables
            float scroll = 0.f;

            // Constructors
            ScrollPane(SNumber x, SNumber y, SNumber w, SNumber h, Panel* parent = nullptr);

            // Functions
            virtual bool handle_event(const Event& event) override;
            virtual void paint(Context& ctx) override;
            
            template<typename T>
            T* add_item(T* panel){
                panel->styleClass = panel->styleClass + " scroll-item";
                panel->clippingEnabled = false;
                items.push_back(std::unique_ptr<Panel>(panel));
                add_child(panel);
                return panel;
            }
        };
    };
};