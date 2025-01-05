#include "draft/interface/widgets/layout.hpp"

namespace Draft::UI {
    const std::vector<const Layout*> Layout::get_children() const {
        return {};
    }

    const Vector2<UnitValue> Layout::get_preferred_size(const Context& ctx) const {
        float baseWidth = style.size.width.get(0.f);
        float baseHeight = style.size.height.get(0.f);

        for(const Layout* child : get_children()){
            auto s = child->get_preferred_size(ctx);
            baseWidth = std::max(s.x.get(0.f), baseWidth);
            baseHeight = std::max(s.y.get(0.f), baseHeight);
        }

        return { baseWidth, baseHeight };
    }
};