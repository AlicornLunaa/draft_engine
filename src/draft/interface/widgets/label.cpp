#include "draft/interface/widgets/label.hpp"

namespace Draft::UI {
    const Vector2<UnitValue> Label::get_preferred_size(const Context& ctx) const {
        Vector2f size = ctx.textBatch.get_text_bounds({ str, font, fontSize });
        return { size.x, size.y };
    }

    void Label::render(Context ctx, std::vector<Command>& commands) const {
        // Render text in the center if the given box
        Command cmd;
        cmd.type = Command::TEXT;
        cmd.x = ctx.bounds.x;
        cmd.y = ctx.bounds.y;
        cmd.color = style.foregroundColor;
        cmd.text.str = str.c_str();
        cmd.text.font = font;
        cmd.text.fontSize = fontSize;
        commands.push_back(cmd);
    };
};