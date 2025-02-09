#include "draft/interface/widgets/grid.hpp"
#include "glm/common.hpp"

namespace Draft::UI {
    const std::vector<const Layout*> Grid::get_children() const {
        std::vector<const Layout*> children;

        for(const Item& item : items){
            children.push_back(item.child);
        }

        return children;
    }

    void Grid::render(Context ctx, std::vector<Command>& commands) const {
        // Render background
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

    void Grid::layout(){
        // Place each child element
        const auto cellWidth = style.size.width / static_cast<float>(columns);
        uint activeColumn = 0;
        float totalHeight = 0.f; // This is used to 'follow' the heights since columns only affect width
        float maxCurrentHeight = 0.f;

        for(Item& item : items){
            const auto width = cellWidth * static_cast<float>(columns);
            Layout& child = *item.child;

            child.style.position.x = cellWidth * static_cast<float>(activeColumn);
            child.style.position.y = totalHeight;
            child.style.size.width = width;

            maxCurrentHeight = Math::max(maxCurrentHeight, child.style.size.height.get(0.f));
            activeColumn += item.columnSpan;

            if(activeColumn > columns){
                // Over the edge, append new row
                activeColumn = 0;
                totalHeight += maxCurrentHeight;
                maxCurrentHeight = 0.f;
            }
        }
    }
};