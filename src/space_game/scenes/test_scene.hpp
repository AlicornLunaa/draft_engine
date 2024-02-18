#pragma once
#include <clydesdale/engine.hpp>

namespace Cl = Clydesdale;

namespace SpaceGame {
    class TestScene : public Clydesdale::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;
        sf::Sprite* sprite;

        Cl::Console console;

        Cl::World world = Cl::World(Cl::Vector2f(0, -10));
        Cl::RigidBody ground;
        Cl::Entity targetEntity;

        Cl::Entity createGravEntity(Cl::AssetManager& assetManager, const Cl::Vector2f position);

    public:
        TestScene(Cl::AssetManager& assetManager, sf::RenderWindow& window);
        ~TestScene();

        void handleEvent(sf::Event event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::Time deltaTime) override;
    };
}
