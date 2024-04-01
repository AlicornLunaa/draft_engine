#pragma once
#include "element.hpp"

namespace Draft {
    class Panel : public Element {
    public:
        Panel();
        Panel(sf::Color backgroundColor, float x, float y, float width, float height);
        Panel(Element& parent, sf::Color backgroundColor, float x, float y, float width, float height);
        ~Panel();

        void clicked();
    };
}