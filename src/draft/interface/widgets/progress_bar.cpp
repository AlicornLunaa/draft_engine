#include "draft/interface/widgets/progress_bar.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "glm/common.hpp"

namespace Draft::UI {
    // Constructor
    ProgressBar::ProgressBar(SNumber x, SNumber y, SNumber w, SNumber h, float* value, Panel* parent) : Panel(parent), value(value) {
        position = {x, y};
        size = {w, h};
        styleClass = "progress-bar";
        progress.styleClass = "progress-bar-complete";
    }

    void ProgressBar::paint(Context& ctx){
        Style style = ctx.stylesheet.get_style(ctx.styleStack + " " + styleClass);

        ctx.batch.draw({
            style.background.value,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {0, 0},
            layer,
            style.backgroundColor.value,
            false
        });

        progress.size.x = size.x.calculate(ctx.parentSize.x) * Math::clamp(*value, 0.f, 1.f);

        Panel::paint(ctx);
    }
};