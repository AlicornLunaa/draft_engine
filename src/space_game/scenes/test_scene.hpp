#pragma once
#include "clydesdale/math/vector2.hpp"
#include <clydesdale/engine.hpp>

namespace Cl = Clydesdale;

namespace SpaceGame {
    class TestScene : public Clydesdale::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;
        sf::Sprite* sprite1;
        sf::Sprite* sprite2;

        Cl::Console console;

        Cl::World world = Cl::World(Cl::Vector2f(0, -10));
        Cl::Entity ground;
        Cl::Entity targetEntity;

        Cl::Entity createGravEntity(Cl::AssetManager& assetManager, const Cl::Vector2f position);
        Cl::Entity createGroundEntity(Cl::AssetManager& assetManager, const Cl::Vector2f position, const Cl::Vector2f size);

    public:
        TestScene(Cl::AssetManager& assetManager, sf::RenderWindow& window);
        ~TestScene();

        void handleEvent(sf::Event event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::Time deltaTime) override;
    };
}
