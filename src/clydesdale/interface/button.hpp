#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "element.hpp"

namespace Clydesdale {
    class Button : public Element {
    public:
        sf::Text label;

        Button();
        Button(sf::Color backgroundColor, const sf::Font& font, const std::string& str, float x, float y, float width, float height);
        Button(Element& parent, sf::Color backgroundColor, const sf::Font& font, const std::string& str, float x, float y, float width, float height);
        ~Button();

        void clicked();
    };
}