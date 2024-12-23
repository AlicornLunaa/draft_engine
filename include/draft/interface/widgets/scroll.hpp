#pragma once

#include "draft/interface/widgets/layout.hpp"

struct Scroll : public Layout {
    // Variables
    static constexpr std::string elementClass = "scroll"; // Permanent, per type
    float scroll = 0.f;

    // Constructors
    using Layout::Layout;

    // Functions
    virtual std::string get_element_class() const override { return elementClass; }
    virtual void build_dom_element(Context& ctx, Element& element) const override;
    virtual void place_child(Context& ctx, Element& parent, Layout& child, Element& element) const override;
};