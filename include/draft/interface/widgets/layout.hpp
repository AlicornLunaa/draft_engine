#pragma once

#include "draft/interface/context.hpp"
#include "draft/interface/dom.hpp"
#include <string>
#include <vector>

/**
 * @brief A layout is a renderable element which renders all its children.
 * There is a 'root' layout which everything else is attached to.
 */
struct Layout {
    // Variables
    static constexpr std::string elementClass = "layout"; // Permanent, per type
    std::vector<std::string> classes; // Stylesheet class name

    Layout* parent = nullptr;
    std::vector<Layout*> children;

    // Constructors
    Layout() = default;
    Layout(const std::vector<std::string>& classes);
    Layout(Layout* parent, const std::vector<std::string>& classes);
    virtual ~Layout() = default;

    // Functions
    virtual void build_dom_element(Context& ctx, Element& element) const;
    virtual void place_child(Context& ctx, Layout& child, Element& element) const;
};