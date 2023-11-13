#pragma once
#include <clydesdale_engine/core.hpp>
#include <clydesdale_engine/util/logger.hpp>

namespace SpaceGame {
    class TestScene : public Clydesdale::Core::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;

        Clydesdale::Core::Entity createGravEntity(Clydesdale::Util::AssetManager& assetManager);

    public:
        TestScene(Clydesdale::Util::AssetManager& assetManager, sf::RenderWindow& window);
            
        void handleEvent(sf::Event event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::Time deltaTime) override;
    };
}
