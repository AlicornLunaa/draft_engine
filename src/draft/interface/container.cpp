#include "draft/interface/container.hpp"
#include "draft/interface/render_command.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/util/color.hpp"
#include "draft/math/glm.hpp"

#include "draft/interface/context.hpp"
#include "draft/interface/dom.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <string>

// Aliasing
template<typename T>
using Opt = std::optional<T>;
using namespace Draft;

// Create 'boxes' or 'render areas' for children for different layouts. A plain layout, the render area is the entire center, minus padding.
// On a scroll the render area is the next available area after a child

// Recursive helpers
void Container::recursive_render(Layout* ptr){
    // Render its children then render the current element
    if(!ptr) return;
    if(!ptr->dom.visibility) return;

    for(RenderCommand& cmd : ptr->dom.renderCommands){
        switch(cmd.type){
        case RenderCommand::DRAW_SPRITE:
            batch.draw({
                cmd.sprite.texture,
                {cmd.sprite.regionX, cmd.sprite.regionY, cmd.sprite.regionWidth, cmd.sprite.regionHeight},
                {cmd.sprite.positionX, cmd.sprite.positionY},
                0.f,
                {cmd.sprite.sizeX, cmd.sprite.sizeY},
                {},
                0.f,
                {cmd.sprite.colorR, cmd.sprite.colorG, cmd.sprite.colorB, cmd.sprite.colorA},
                false
            });
            break;

        case RenderCommand::DRAW_TEXT:
            textBatch.draw_text(batch, {
                std::string(cmd.text.string, cmd.text.stringLength),
                cmd.text.font,
                (uint)cmd.text.fontSize,
                {cmd.text.positionX, cmd.text.positionY},
                {cmd.text.originX, cmd.text.originY},
                {cmd.text.colorR, cmd.text.colorG, cmd.text.colorB, cmd.text.colorA},
                0.f,
                1.f
            });
            break;

        case RenderCommand::BEGIN_SCISSOR:
            batch.flush();
            scissor.box.x = cmd.scissor.x;
            scissor.box.y = cmd.scissor.y;
            scissor.box.width = cmd.scissor.width;
            scissor.box.height = cmd.scissor.height;
            scissor.begin();
            break;

        case RenderCommand::END_SCISSOR:
            scissor.end();
            batch.flush();
            break;
        }
    }

    for(Layout* child : ptr->children){
        recursive_render(child);
    }
}

void recursive_debug(ShapeBatch& batch, Element* ptr){
    // Render its children then render the current element
    if(!ptr) return;
    if(!ptr->visibility) return;

    for(RenderCommand& cmd : ptr->renderCommands){
        switch(cmd.type){
        case RenderCommand::DRAW_SPRITE:
            batch.draw_rect(Vector2f(cmd.sprite.positionX, cmd.sprite.positionY), {cmd.sprite.sizeX, cmd.sprite.sizeY}, 0.f);
            break;

        case RenderCommand::BEGIN_SCISSOR:
            batch.draw_rect(Vector2f(cmd.scissor.x, cmd.scissor.y), {cmd.scissor.width, cmd.scissor.height}, 0.f);
            break;

        case RenderCommand::DRAW_TEXT:
        case RenderCommand::END_SCISSOR:
            break;
        }
    }

    // for(Layout* child : ptr->children){
    //     // recursive_debug(batch, &child);
    // }
}

// Functions
void Container::recursive_build_dom(Context ctx, Layout* ptr, Element* dom){
    // Base case for ending the tree
    if(!ptr) return;
}

void Container::recursive_layout_add(Layout* parent, const LayoutSkeleton& skel){
    // Add this layout as a child of parent
    assert(skel.layout != nullptr && "Layout cant be null");
    m_allElements.push_back(std::unique_ptr<Layout>(skel.layout));
    skel.layout->parent = parent;
    parent->children.push_back(skel.layout);

    for(auto& child : skel.children){
        recursive_layout_add(skel.layout, child);
    }
}

Layout* Container::add_layout(LayoutSkeleton skel){
    // Add this layout as a child of parent
    recursive_layout_add(&root, skel);
    return skel.layout;
}

void Container::build_dom(RenderWindow& window){
    // Build a DOM tree out of the layout supplied
    Vector2f winSize = window.get_size();
    Context ctx{{0, 0, winSize.x, winSize.y}};

    // Create the dom element with current data
    root.generate_dom(ctx, stylesheet);

    // Build sprites n stuff per layout type
    root.generate_render_commands();
}

void Container::render(RenderWindow& window){
    // Render each layer
    Vector2f winSize = window.get_size();
    OrthoCamera camera = {
        { 0, 0, 10 },
        { 0, 0, -1 },
        0, winSize.x,
        winSize.y, 0,
        0.1f, 100.f
    };

    batch.set_proj_matrix(camera.get_combined());
    batch.begin();
    recursive_render(&root);
    batch.end();
}

void Container::debug(RenderWindow& window){
    // Render each layer
    Vector2f winSize = window.get_size();
    OrthoCamera camera = {
        { 0, 0, 10 },
        { 0, 0, -1 },
        0, 1280,
        0, 720,
        0.1f, 100.f
    };

    shapeBatch.set_proj_matrix(camera.get_combined());
    shapeBatch.set_color(Color::LIME);
    shapeBatch.begin();
    // recursive_debug(shapeBatch, &root);
    shapeBatch.end();
}