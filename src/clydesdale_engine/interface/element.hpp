#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

namespace Clydesdale {
    namespace Interface {

        class Element : public sf::Drawable {
        private:
            Element* parent = nullptr;
            std::vector<Element*> children;

            virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

        public:
            sf::RectangleShape shape;

            Element();
            Element(float x, float y, float width, float height);
            Element(Element& parent, float x, float y, float width, float height);
            ~Element();

            void update(sf::RenderWindow& window, float deltaTime, sf::Transform transform = sf::Transform::Identity);
        };

    }
}