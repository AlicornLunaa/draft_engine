#pragma once

#include "draft/interface/widgets/label.hpp"
#include "draft/interface/widgets/layout.hpp"

namespace Draft {
    namespace UI {
        struct Button : public Layout {
        public:
            Color hoverColor = Color::LIGHT_GRAY;
            Color clickColor = Color::GREEN;
            std::function<void(void)> onClick;
            Label label;

            mutable bool* clicked = nullptr;
            mutable bool hovered = false;

            virtual const std::vector<const Layout*> get_children() const override;
            virtual void render(Context ctx, std::vector<Command>& commands) const override;
        };
    };
};