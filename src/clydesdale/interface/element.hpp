#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

namespace Clydesdale {
    class Element : public sf::Drawable {
    private:
        Element* parent = nullptr;
        std::vector<Element*> children;

        bool isHovered = false;
        bool isDragging = false;

        virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

    public:
        sf::RectangleShape shape;

        Element();
        Element(float x, float y, float width, float height);
        Element(Element& parent, float x, float y, float width, float height);
        ~Element();

        virtual void clicked();
        void update(sf::RenderWindow& window, float deltaTime, sf::Transform transform = sf::Transform::Identity);
    };
}