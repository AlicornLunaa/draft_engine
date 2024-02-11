#include "element.hpp"

namespace Clydesdale {
    Element::Element() {
        shape = sf::RectangleShape({ 0, 0 });
    }

    Element::Element(float x, float y, float width, float height) : Element() {
        shape.setSize({ width, height });
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::White);
    }

    Element::Element(Element& parent, float x, float y, float width, float height) : Element(x, y, width, height) {
        this->parent = &parent;
        parent.children.push_back(this);
    }

    Element::~Element(){}

    void Element::clicked(){}

    void Element::update(sf::RenderWindow& window, float deltaTime, sf::Transform transform){
        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        sf::FloatRect rect = shape.getGlobalBounds();

        if(parent){
            // Parent exists, move the transform for it
            transform *= parent->shape.getTransform();
        }

        rect = transform.transformRect(rect);
        
        if(rect.contains({ (float)mouse.x, (float)mouse.y })){
            clicked();
        }

        // Update children
        for(auto* ptr : children){
            ptr->update(window, deltaTime, transform);
        }
    }

    void Element::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        // Draw main element
        sf::RenderStates newState = sf::RenderStates(states.transform * shape.getTransform());
        target.draw(shape, states);

        // Draw children
        for(auto* ptr : children){
            target.draw(*ptr, newState);
        }
    }
}