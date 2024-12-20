#pragma once

#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/interface/v2/layout.hpp"
#include "draft/interface/v2/style.hpp"
#include "draft/interface/v2/dom.hpp"
#include <memory>
#include <vector>

class Container {
private:
    std::vector<std::unique_ptr<Layout>> m_allElements; // Holds all the elements to manage its memory
    Draft::OrthoCamera camera{{ 0, 0, 10 }, { 0, 0, -1 }, 0, 1280, 0, 720, 0.1f, 100.f};
    Draft::TextRenderer textBatch;

    void recursive_build_dom(Layout* ptr, Element* dom);

public:
    Stylesheet stylesheet;
    Layout root = {{"#root"}};
    std::unique_ptr<Element> dom = nullptr;

    template<typename T>
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

    void build_dom();
    void render(Draft::SpriteBatch& batch);
};