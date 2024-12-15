#pragma once

#include "draft/interface/panel.hpp"
#include "draft/interface/rectangle.hpp"
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
            ScrollPane(float x, float y, float w, float h, Panel* parent = nullptr);

            // Functions
            virtual bool handle_event(const Event& event) override;
            virtual void paint(Context& ctx) override;
            
            template<typename T>
            T* add_item(T* panel){
                items.push_back(std::unique_ptr<Panel>(panel));
                add_child(panel);
                return panel;
            }
        };
    };
};