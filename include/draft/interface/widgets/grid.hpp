#pragma once

#include "draft/interface/widgets/layout.hpp"

namespace Draft {
    namespace UI {
        struct Grid : public Layout {
            // Data structures
            struct Item {
                Layout* child = nullptr;
                uint columnSpan = 1;
            };

            // Lays children out in rows and columns, autosizing is not supported
            mutable std::vector<Item> items;
            uint columns = 12;

            virtual const std::vector<const Layout*> get_children() const override;
            virtual void render(Context ctx, std::vector<Command>& commands) const override;
            virtual void layout() override;
        };
    };
};