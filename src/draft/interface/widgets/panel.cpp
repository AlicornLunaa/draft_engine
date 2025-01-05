#include "draft/interface/widgets/panel.hpp"

namespace Draft::UI {
    const std::vector<const Layout*> Panel::get_children() const {
        return children;
    }

    void Panel::render(Context ctx, std::vector<Command>& commands) const {
        Command cmd;
        cmd.type = Command::SPRITE;
        cmd.x = ctx.bounds.x;
        cmd.y = ctx.bounds.y;
        cmd.color = style.backgroundColor;
        cmd.sprite.width = ctx.bounds.width;
        cmd.sprite.height = ctx.bounds.height;
        cmd.sprite.texture = nullptr;
        commands.push_back(cmd);
    }
};