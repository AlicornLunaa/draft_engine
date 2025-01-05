#pragma once

#include "draft/interface/global_style.hpp"
#include "draft/interface/command.hpp"
#include "draft/interface/context.hpp"
#include <vector>

namespace Draft {
    namespace UI {
        struct Layout {
        public:
            GlobalStyle style;

            virtual const std::vector<const Layout*> get_children() const;
            virtual const Vector2<UnitValue> get_preferred_size(const Context& ctx) const;
            virtual void render(Context ctx, std::vector<Command>& commands) const = 0;
        };
    };
};