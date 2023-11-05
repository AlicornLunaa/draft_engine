#include "panel.hpp"
#include <iostream>
using namespace Clydesdale::Interface;

Panel::Panel() : Element() {}

Panel::Panel(sf::Color backgroundColor, float x, float y, float width, float height) : Element(x, y, width, height) {
    shape.setFillColor(backgroundColor);
}

Panel::Panel(Element& parent, sf::Color backgroundColor, float x, float y, float width, float height) : Element(parent, x, y, width, height) {
    shape.setFillColor(backgroundColor);
}

Panel::~Panel(){}

void Panel::clicked(){
}