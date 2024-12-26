#include "draft/interface/widgets/layout.hpp"
#include "draft/interface/context.hpp"
#include "draft/interface/unit_value.hpp"
#include "draft/interface/style.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"

// Aliasing
template<typename T>
using Opt = std::optional<T>;
using namespace Draft;

// Constructors
Layout::Layout(const std::vector<std::string>& classes) : classes(classes) {}

Layout::Layout(Layout* parent, const std::vector<std::string>& classes) : Layout(classes){
    parent->children.push_back(this);
    this->parent = parent;
}

// Functions
void Layout::generate_render_commands(){
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

void Layout::generate_dom(Context ctx, Stylesheet& stylesheet){
    // Since theres a layout, decide its properties the dom should replicate
    UnitValue left = stylesheet.get(*this, "left", UnitValue(), NONE);
    UnitValue top = stylesheet.get(*this, "top", UnitValue(), NONE);
    UnitValue right = stylesheet.get(*this, "right", UnitValue(), NONE);
    UnitValue bottom = stylesheet.get(*this, "bottom", UnitValue(), NONE);

    UnitValue paddingLeft = stylesheet.get(*this, "padding-left", UnitValue(), NONE);
    UnitValue paddingTop = stylesheet.get(*this, "padding-top", UnitValue(), NONE);
    UnitValue paddingRight = stylesheet.get(*this, "padding-right", UnitValue(), NONE);
    UnitValue paddingBottom = stylesheet.get(*this, "padding-bottom", UnitValue(), NONE);

    UnitValue marginLeft = stylesheet.get(*this, "margin-left", UnitValue(), NONE);
    UnitValue marginTop = stylesheet.get(*this, "margin-top", UnitValue(), NONE);
    UnitValue marginRight = stylesheet.get(*this, "margin-right", UnitValue(), NONE);
    UnitValue marginBottom = stylesheet.get(*this, "margin-bottom", UnitValue(), NONE);

    OptUnitValue maxWidth = stylesheet.get<UnitValue>(*this, "max-width", NONE);
    OptUnitValue minWidth = stylesheet.get<UnitValue>(*this, "min-width", NONE);
    OptUnitValue width = stylesheet.get<UnitValue>(*this, "width", NONE);
    OptUnitValue maxHeight = stylesheet.get<UnitValue>(*this, "max-height", NONE);
    OptUnitValue minHeight = stylesheet.get<UnitValue>(*this, "min-height", NONE);
    OptUnitValue height = stylesheet.get<UnitValue>(*this, "height", NONE);

    bool visible = stylesheet.get(*this, "visibility", true, NONE); 
    Color color = stylesheet.get(*this, "color", Color::BLACK, NONE);
    Color backgroundColor = stylesheet.get(*this, "background-color", Color::WHITE, NONE);
    Opt<std::string> backgroundImage = stylesheet.get<std::string>(*this, "background-image", NONE);
    float opacity = stylesheet.get(*this, "opacity", 1.f, NONE);

    float fontSize = stylesheet.get(*this, "font-size", 20.f, NONE);
    Opt<std::string> fontStyle = stylesheet.get<std::string>(*this, "font-style", NONE);

    // Do assignments for the dom element
    dom.dimensions = Vector4f(left.get(ctx.parentBounds.width), top.get(ctx.parentBounds.height), right.get(ctx.parentBounds.width), bottom.get(ctx.parentBounds.height));
    dom.margin = Vector4f(marginLeft.get(ctx.parentBounds.width), marginTop.get(ctx.parentBounds.height), marginRight.get(ctx.parentBounds.width), marginBottom.get(ctx.parentBounds.height));
    dom.padding = Vector4f(paddingLeft.get(ctx.parentBounds.width), paddingTop.get(ctx.parentBounds.height), paddingRight.get(ctx.parentBounds.width), paddingBottom.get(ctx.parentBounds.height));
    color.a *= opacity;
    backgroundColor.a *= opacity;

    dom.fontSize = fontSize;

    // Do positional magic to convert CSS stuff to OpenGL batching
    // Colors ig
    dom.color = color;
    dom.backgroundColor = backgroundColor;
    dom.visibility = visible;
    dom.texture = nullptr;
    dom.font = nullptr;

    if(backgroundImage.has_value()){
        dom.texture = Assets::manager.get<Texture>(backgroundImage.value(), true);
    }

    if(fontStyle.has_value()){
        dom.font = Assets::manager.get<Font>(fontStyle.value(), true);
    }

    // Sizing, priority order goes content-size => left/right => width
    dom.size.x = Math::abs(dom.dimensions.z - dom.dimensions.x);
    dom.size.y = Math::abs(dom.dimensions.w - dom.dimensions.y);

    if(width.has_value()) dom.size.x = width.value().get(ctx.parentBounds.width);
    if(minWidth.has_value()) dom.size.x = Math::max(dom.size.x, minWidth.value().get(ctx.parentBounds.width));
    if(maxWidth.has_value()) dom.size.x = Math::min(dom.size.x, maxWidth.value().get(ctx.parentBounds.width));
    if(height.has_value()) dom.size.y = height.value().get(ctx.parentBounds.height);
    if(minHeight.has_value()) dom.size.y = Math::max(dom.size.y, minHeight.value().get(ctx.parentBounds.height));
    if(maxHeight.has_value()) dom.size.y = Math::min(dom.size.y, maxHeight.value().get(ctx.parentBounds.height));

    // Positioning
    dom.position = {left.get(ctx.parentBounds.width), top.get(ctx.parentBounds.height)};
    dom.position.x += dom.margin.x + ctx.parentBounds.x;
    dom.position.y += dom.margin.y + ctx.parentBounds.y;
    dom.size.x -= dom.margin.x + dom.margin.z;
    dom.size.y -= dom.margin.y + dom.margin.w;

    dom.bounds = {
        dom.position.x, dom.position.y,
        dom.size.x, dom.size.y,
    };

    // Update rendering context for children
    ctx.parentBounds.x = dom.position.x + dom.padding.x;
    ctx.parentBounds.y = dom.position.y + dom.padding.y;
    ctx.parentBounds.width = dom.size.x - dom.padding.z * 2;
    ctx.parentBounds.height = dom.size.y - dom.padding.w * 2;

    // Build its children with the new context
    for(Layout* child : children){
        // Add a layout positioning function for each layout type which places each child
        child->generate_dom(ctx, stylesheet);
    }
}