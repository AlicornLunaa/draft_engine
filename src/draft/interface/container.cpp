#include "draft/interface/container.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/util/color.hpp"
#include "draft/math/glm.hpp"

#include "draft/interface/context.hpp"
#include "draft/interface/dom.hpp"
#include "draft/interface/unit_value.hpp"

#include <cassert>
#include <memory>
#include <optional>

template<typename T>
using Opt = std::optional<T>;
using namespace Draft;

static constexpr Vector2f MAX_SIZE(1280, 720);

void Container::recursive_build_dom(Layout* ptr, Element* dom){
    // Base case for ending the tree
    if(!ptr) return;

    // Create a rendering context from parent
    Context ctx{ {0, 0, MAX_SIZE.x, MAX_SIZE.y}, MAX_SIZE};
    if(dom->parent){
        // Update context
        ctx.parentBounds.x = dom->parent->position.x + dom->parent->padding.x;
        ctx.parentBounds.y = dom->parent->position.y + dom->parent->padding.y;
        ctx.parentBounds.width = dom->parent->size.x - dom->parent->padding.z * 2;
        ctx.parentBounds.height = dom->parent->size.y - dom->parent->padding.w * 2;
    }

    // Since theres a layout, decide its properties the dom should replicate
    UnitValue left = stylesheet.get(*ptr, "left", UnitValue(), NONE);
    UnitValue top = stylesheet.get(*ptr, "top", UnitValue(), NONE);
    UnitValue right = stylesheet.get(*ptr, "right", UnitValue(), NONE);
    UnitValue bottom = stylesheet.get(*ptr, "bottom", UnitValue(), NONE);

    UnitValue paddingLeft = stylesheet.get(*ptr, "padding-left", UnitValue(), NONE);
    UnitValue paddingTop = stylesheet.get(*ptr, "padding-top", UnitValue(), NONE);
    UnitValue paddingRight = stylesheet.get(*ptr, "padding-right", UnitValue(), NONE);
    UnitValue paddingBottom = stylesheet.get(*ptr, "padding-bottom", UnitValue(), NONE);

    UnitValue marginLeft = stylesheet.get(*ptr, "margin-left", UnitValue(), NONE);
    UnitValue marginTop = stylesheet.get(*ptr, "margin-top", UnitValue(), NONE);
    UnitValue marginRight = stylesheet.get(*ptr, "margin-right", UnitValue(), NONE);
    UnitValue marginBottom = stylesheet.get(*ptr, "margin-bottom", UnitValue(), NONE);

    OptUnitValue maxWidth = stylesheet.get<UnitValue>(*ptr, "max-width", NONE);
    OptUnitValue minWidth = stylesheet.get<UnitValue>(*ptr, "min-width", NONE);
    OptUnitValue width = stylesheet.get<UnitValue>(*ptr, "width", NONE);
    OptUnitValue maxHeight = stylesheet.get<UnitValue>(*ptr, "max-height", NONE);
    OptUnitValue minHeight = stylesheet.get<UnitValue>(*ptr, "min-height", NONE);
    OptUnitValue height = stylesheet.get<UnitValue>(*ptr, "height", NONE);

    bool visible = stylesheet.get(*ptr, "visibility", true, NONE); 
    Color color = stylesheet.get(*ptr, "color", Color::BLACK, NONE);
    Color backgroundColor = stylesheet.get(*ptr, "background-color", Color::WHITE, NONE);
    Opt<std::string> backgroundImage = stylesheet.get<std::string>(*ptr, "background-image", NONE);
    float opacity = stylesheet.get(*ptr, "opacity", 1.f, NONE);

    float fontSize = stylesheet.get(*ptr, "font-size", 20.f, NONE);
    Opt<std::string> fontStyle = stylesheet.get<std::string>(*ptr, "font-style", NONE);

    // Do assignments for the dom element
    dom->dimensions = Vector4f(left.get(ctx.parentBounds.width), top.get(ctx.parentBounds.height), right.get(ctx.parentBounds.width), bottom.get(ctx.parentBounds.height));
    dom->margin = Vector4f(marginLeft.get(ctx.parentBounds.width), marginTop.get(ctx.parentBounds.height), marginRight.get(ctx.parentBounds.width), marginBottom.get(ctx.parentBounds.height));
    dom->padding = Vector4f(paddingLeft.get(ctx.parentBounds.width), paddingTop.get(ctx.parentBounds.height), paddingRight.get(ctx.parentBounds.width), paddingBottom.get(ctx.parentBounds.height));
    color.a *= opacity;
    backgroundColor.a *= opacity;

    dom->fontSize = fontSize;

    // Do positional magic to convert CSS stuff to OpenGL batching
    // Colors ig
    dom->color = color;
    dom->backgroundColor = backgroundColor;
    dom->visibility = visible;
    dom->texture = nullptr;
    dom->font = nullptr;

    if(backgroundImage.has_value()){
        dom->texture = Assets::manager.get<Texture>(backgroundImage.value(), true);
    }

    if(fontStyle.has_value()){
        dom->font = Assets::manager.get<Font>(fontStyle.value(), true);
    }

    // Sizing, priority order goes content-size => left/right => width
    dom->size.x = Math::abs(dom->dimensions.z - dom->dimensions.x);
    dom->size.y = Math::abs(dom->dimensions.w - dom->dimensions.y);

    if(width.has_value()) dom->size.x = width.value().get(ctx.parentBounds.width);
    if(minWidth.has_value()) dom->size.x = Math::max(dom->size.x, minWidth.value().get(ctx.parentBounds.width));
    if(maxWidth.has_value()) dom->size.x = Math::min(dom->size.x, maxWidth.value().get(ctx.parentBounds.width));
    if(height.has_value()) dom->size.y = height.value().get(ctx.parentBounds.height);
    if(minHeight.has_value()) dom->size.y = Math::max(dom->size.y, minHeight.value().get(ctx.parentBounds.height));
    if(maxHeight.has_value()) dom->size.y = Math::min(dom->size.y, maxHeight.value().get(ctx.parentBounds.height));

    // Positioning
    dom->position = {left.get(ctx.parentBounds.width), top.get(ctx.parentBounds.height)};
    dom->position.x += dom->margin.x + ctx.parentBounds.x;
    dom->position.y += dom->margin.y + ctx.parentBounds.y;
    dom->size.x -= dom->margin.x + dom->margin.z;
    dom->size.y -= dom->margin.y + dom->margin.w;

    dom->bounds = {
        dom->position.x, dom->position.y,
        dom->size.x, dom->size.y,
    };

    // Build its children
    for(Layout* child : ptr->children){
        // Add a layout positioning function for each layout type which places each child
        dom->children.push_back({});
        dom->children.back().parent = dom;
        recursive_build_dom(child, &dom->children.back());
    }

    // Build sprites n stuff per layout type
    if(ptr->parent){
        ptr->parent->place_child(ctx, *dom->parent, *ptr, *dom);
    }
    ptr->build_dom_element(ctx, *dom);
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

void Container::build_dom(){
    // Build a DOM tree out of the layout supplied
    dom = std::unique_ptr<Element>(new Element());
    recursive_build_dom(&root, dom.get());
}

void recursive_render(SpriteBatch& batch, TextRenderer& textBatch, Element* ptr){
    // Render its children then render the current element
    if(!ptr) return;
    if(!ptr->visibility) return;

    for(Element& child : ptr->children){
        recursive_render(batch, textBatch, &child);
    }

    for(auto& text : ptr->text){
        textBatch.draw_text(batch, text);
    }

    for(auto& sprite : ptr->sprites){
        batch.draw(sprite);
    }
}

void Container::render(SpriteBatch& batch){
    // Render each layer
    batch.set_proj_matrix(camera.get_combined());
    recursive_render(batch, textBatch, dom.get());
}

void recursive_debug(ShapeBatch& batch, Element* ptr){
    // Render its children then render the current element
    if(!ptr) return;
    if(!ptr->visibility) return;

    for(Element& child : ptr->children){
        recursive_debug(batch, &child);
    }

    for(auto& sprite : ptr->sprites){
        batch.draw_rect(sprite.position - Vector2f{0, sprite.size.y}, sprite.size, 0.f);
    }
}

void Container::debug(ShapeBatch& batch){
    // Render each layer
    batch.set_proj_matrix(camera.get_combined());
    batch.set_color(Color::LIME);
    recursive_debug(batch, dom.get());
}