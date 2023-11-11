#pragma once
#include <clydesdale_engine/core.hpp>

namespace SpaceGame {
    class TestScene : public Clydesdale::Core::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;

    public:
        TestScene(Clydesdale::Util::AssetManager& assetManager, sf::RenderWindow& window);
            
        void handleEvent(sf::Event event);
        void update(sf::Time deltaTime);
        void render(sf::Time deltaTime);
    };
}
