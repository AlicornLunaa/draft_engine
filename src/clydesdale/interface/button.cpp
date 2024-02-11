#include "button.hpp"
#include <iostream>

namespace Clydesdale {
    Button::Button() : Element() {}

    Button::Button(sf::Color backgroundColor, const sf::Font& font, const std::string& str, float x, float y, float width, float height) : Element(x, y, width, height), label(str, font) {
        shape.setFillColor(backgroundColor);
    }

    Button::Button(Element& parent, sf::Color backgroundColor, const sf::Font& font, const std::string& str, float x, float y, float width, float height) : Element(parent, x, y, width, height), label(str, font) {
        shape.setFillColor(backgroundColor);
    }

    Button::~Button(){}

    void Button::clicked(){
        std::cout << "panel touched\n";
    }
}
