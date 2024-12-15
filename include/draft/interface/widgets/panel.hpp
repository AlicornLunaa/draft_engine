#pragma once

#include "draft/input/event.hpp"
#include "draft/interface/context.hpp"
#include "draft/interface/styled_number.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/clip.hpp"

#include <vector>

namespace Draft {
    namespace UI {
        class Panel {
        private:
            // Variables
            std::vector<Panel*> children;
            Panel* parent = nullptr;
            Clip scissor; // Used to prevent rendering off of the panel

        protected:
            // Variables to be modified by children
            FloatRect bounds;
            float layer = -10.f;

            // Protected functions
            const Panel* get_parent() const { return parent; }
            void update_state(Context& ctx);
            virtual void preprocess_children(Context& ctx){}; // Used for modifying stuff before the children are rendered

        public:
            // Variables
            Vector2<SNumber> position{0.0_pixels};
            Vector2<SNumber> size{0.0_pixels};
            Style* stylesheet = nullptr;

            // Constructors
            Panel(Panel* parent = nullptr);
            virtual ~Panel() = default;

            // Friends
            friend class Container;

            // Functions
            virtual bool handle_event(const Event& event);;
            virtual void paint(Context& ctx);
            void add_child(Panel* ptr);
            void remove_child(Panel* ptr);
            const FloatRect& get_bounds() const { return bounds; }
        };
    }
};