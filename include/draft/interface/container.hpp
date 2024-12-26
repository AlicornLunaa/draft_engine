#pragma once

#include "draft/rendering/batching/shape_batch.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/interface/style.hpp"
#include "draft/interface/dom.hpp"
#include "draft/interface/widgets/layout.hpp"
#include "draft/rendering/clip.hpp"
#include "draft/rendering/render_window.hpp"
#include <memory>
#include <vector>

struct LayoutSkeleton {
    Layout* layout = nullptr;
    std::vector<LayoutSkeleton> children = {};

    LayoutSkeleton() = default;
    LayoutSkeleton(Layout* ptr) : layout(ptr) {}
    LayoutSkeleton(Layout* ptr, const std::vector<LayoutSkeleton>& skel) : layout(ptr), children(skel) {}
};

class Container {
private:
    // Private variables
    std::vector<std::unique_ptr<Layout>> m_allElements; // Holds all the elements to manage its memory
    Draft::TextRenderer textBatch;
    Draft::SpriteBatch batch;
    Draft::ShapeBatch shapeBatch;
    Draft::Clip scissor;

    // Private functions
    void recursive_render(Layout* ptr);
    void recursive_build_dom(Context ctx, Layout* ptr, Element* dom);
    void recursive_layout_add(Layout* parent, const LayoutSkeleton& skel);

public:
    // Public variables
    Stylesheet stylesheet;
    Layout root = {{"#root"}};

    // Functions
    Layout* add_layout(LayoutSkeleton skel);
    void build_dom(Draft::RenderWindow& window);
    void render(Draft::RenderWindow& window);
    void debug(Draft::RenderWindow& window);
};