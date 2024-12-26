#include "draft/interface/widgets/scroll.hpp"
#include "draft/interface/render_command.hpp"

void Scroll::generate_render_commands(){
    // Render self box
    RenderCommand cmd;
    cmd.type = RenderCommand::DRAW_SPRITE;
    cmd.sprite.texture = dom.texture;
    cmd.sprite.regionX = 0;
    cmd.sprite.regionY = 0;
    cmd.sprite.regionWidth = 0;
    cmd.sprite.regionHeight = 0;
    cmd.sprite.positionX = dom.position.x;
    cmd.sprite.positionY = dom.position.y;
    cmd.sprite.sizeX = dom.size.x;
    cmd.sprite.sizeY = dom.size.y;
    cmd.sprite.colorR = dom.backgroundColor.r;
    cmd.sprite.colorG = dom.backgroundColor.g;
    cmd.sprite.colorB = dom.backgroundColor.b;
    cmd.sprite.colorA = dom.backgroundColor.a;
    dom.renderCommands.push_back(cmd);

    // Render all children here, then move them to be in line with this layout so its a 'container'
    for(uint index = 0; index < children.size(); index++){
        Layout* childLayout = children[index];
        childLayout->generate_render_commands();
    }
}