#include "draft/interface/widgets/button.hpp"

namespace Draft::UI {
    const std::vector<const Layout*> Button::get_children() const {
        return { &label };
    }

    void Button::render(Context ctx, std::vector<Command>& commands) const {
        hovered = Math::contains(ctx.bounds, ctx.app->mouse.get_position());
        if(clicked) *clicked = hovered && ctx.app->mouse.is_pressed(Mouse::LEFT_BUTTON);

        if(onClick && hovered && ctx.app->mouse.is_just_pressed(Mouse::LEFT_BUTTON)){
            onClick();
        }

        Color color = style.backgroundColor;
        if(hovered) color = hoverColor;
        if(clicked && *clicked) color = clickColor;

        Command cmd;
        cmd.type = Command::SPRITE;
        cmd.x = ctx.bounds.x;
        cmd.y = ctx.bounds.y;
        cmd.color = color;
        cmd.sprite.width = ctx.bounds.width;
        cmd.sprite.height = ctx.bounds.height;
        cmd.sprite.texture = nullptr;
        commands.push_back(cmd);
    };
};