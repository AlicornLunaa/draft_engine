#pragma once

#include "draft/interface/widgets/layout.hpp"

namespace Draft {
    namespace UI {
        struct Label : public Layout {
        public:
            std::string str = "Hello World!";
            Font* font = nullptr;
            uint fontSize = 22;

            virtual const Vector2<UnitValue> get_preferred_size(const Context& ctx) const override;
            virtual void render(Context ctx, std::vector<Command>& commands) const override;
        };
    };
};