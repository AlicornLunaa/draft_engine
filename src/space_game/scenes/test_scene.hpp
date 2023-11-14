#pragma once
#include <clydesdale/engine.hpp>

namespace SpaceGame {
    class TestScene : public Clyde::Core::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;

        Clyde::Core::Entity createGravEntity(Clyde::Util::AssetManager& assetManager, const sf::Vector2f position);

    public:
        TestScene(Clyde::Util::AssetManager& assetManager, sf::RenderWindow& window);
            
        void handleEvent(sf::Event event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::Time deltaTime) override;
    };
}
