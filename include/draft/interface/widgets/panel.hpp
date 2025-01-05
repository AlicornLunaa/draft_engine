#pragma once

#include "draft/interface/widgets/layout.hpp"

namespace Draft {
    namespace UI {
        struct Panel : public Layout {
        public:
            // Lays children out absolutely within this container
            std::vector<const Layout*> children;

            virtual const std::vector<const Layout*> get_children() const override;
            virtual void render(Context ctx, std::vector<Command>& commands) const override;
        };
    };
};