#include "element.hpp"
#include <iostream>
using namespace Clydesdale::Interface;

Element::Element() {
    shape = sf::RectangleShape({ 0, 0 });
}

Element::Element(float x, float y, float width, float height) : Element() {
    shape.setSize({ width, height });
    shape.setPosition(x, y);
    shape.setFillColor(sf::Color::White);
}

Element::Element(Element& parent, float x, float y, float width, float height) : Element(x, y, width, height) {
    parent.children.push_back(this);
}

Element::~Element(){}

void Element::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    // Draw main element
    sf::RenderStates newState = sf::RenderStates(shape.getTransform());
    target.draw(shape, states);

    // Draw children
    for(auto* ptr : children){
        target.draw(*ptr, newState);
    }
}