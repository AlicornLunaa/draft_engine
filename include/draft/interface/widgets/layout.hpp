#pragma once

#include "draft/interface/context.hpp"
#include "draft/interface/dom.hpp"
#include <string>
#include <vector>

class Stylesheet;

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
    Element dom; // Generic element to hold all render information

    // Constructors
    Layout() = default;
    Layout(const std::vector<std::string>& classes);
    Layout(Layout* parent, const std::vector<std::string>& classes);
    virtual ~Layout() = default;

    // Functions
    virtual std::string get_element_class() const { return elementClass; }
    virtual void generate_render_commands();
    virtual void generate_dom(Context ctx, Stylesheet& stylesheet);
};