#pragma once

#include "draft/rendering/batching/shape_batch.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/interface/style.hpp"
#include "draft/interface/dom.hpp"
#include "draft/interface/widgets/layout.hpp"
#include <memory>
#include <type_traits>
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
    std::vector<std::unique_ptr<Layout>> m_allElements; // Holds all the elements to manage its memory
    Draft::OrthoCamera camera{{ 0, 0, 10 }, { 0, 0, -1 }, 0, 1280, 0, 720, 0.1f, 100.f};
    Draft::TextRenderer textBatch;

    void recursive_build_dom(Layout* ptr, Element* dom);
    void recursive_layout_add(Layout* parent, const LayoutSkeleton& skel);

public:
    Stylesheet stylesheet;
    Layout root = {{"#root"}};
    std::unique_ptr<Element> dom = nullptr;

    template<typename T, typename = std::enable_if_t<std::is_base_of_v<Layout, T>>>
    T* add_layout(Layout& parent, T* child){
        // Add this layout as a child of parent
        assert(child != nullptr && "Child cant be null");
        m_allElements.push_back(std::unique_ptr<T>(child));

        if(child->parent){
            auto& children = child->parent->children;
            auto iter = std::find(children.begin(), children.end(), child);
            children.erase(iter);
            child->parent = nullptr;
        }

        parent.children.push_back(child);
        child->parent = &parent;

        return child;
    }

    Layout* add_layout(LayoutSkeleton skel){
        // Add this layout as a child of parent
        recursive_layout_add(&root, skel);
        return skel.layout;
    }

    void build_dom();
    void render(Draft::SpriteBatch& batch);
    void debug(Draft::ShapeBatch& batch);
};