#include "draft/interface/widgets/grid.hpp"

namespace Draft::UI {
    void Grid::render(Context ctx, std::vector<Command>& commands) const {
        // Get the size and position of the inner box
        // auto [content, outer] = get_content_box(ctx);

        // Command cmd;
        // cmd.type = Command::SPRITE;
        // cmd.x = content.x;
        // cmd.y = content.y;
        // cmd.color = style.backgroundColor;
        // cmd.sprite.width = content.width;
        // cmd.sprite.height = content.height;
        // cmd.sprite.texture = nullptr;
        // commands.push_back(cmd);

        // // Calculate children render commands
        // size_t startCount = commands.size();
        // float cellWidth = content.width / ((float)columns);
        // uint currentColumn = 0;

        // for(const Item& item : items){
        //     // Prevent rendering off the grid
        //     if(currentColumn > columns) break;

        //     // Calculate content size
        //     float width = cellWidth * item.columnSpan;
        //     currentColumn += item.columnSpan;

        //     // Create context for this element
        //     Context newCtx(ctx.batch, ctx.textBatch, {0, 0, content.width, content.height});

        //     // Obtain render commands for children
        //     item.child->render(newCtx, commands);
        // }

        // size_t count = commands.size() - startCount;

        // // With the count of objects which are the children commands, move the child to be relative to this layout object
        // Vector2f topLeftPadding(style.padding.left.get(ctx.bounds.width), style.padding.top.get(ctx.bounds.width));

        // for(size_t i = 0; i < count; i++){
        //     Command& cmd = commands[i + startCount];
        //     cmd.x += content.x + topLeftPadding.x; // Padding is needed here to push it off the side
        //     cmd.y += content.y + topLeftPadding.y;
        // }
    };
};