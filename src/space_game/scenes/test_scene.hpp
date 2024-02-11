#pragma once
#include <clydesdale/engine.hpp>

namespace SpaceGame {
    class TestScene : public Clydesdale::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;
        sf::Sprite* sprite;

        Clydesdale::Console console;

        Clydesdale::World world = Clydesdale::World(Clydesdale::Vector2f(0, 0));
        Clydesdale::RigidBody ground;
        Clydesdale::RigidBody body;
        Clydesdale::Entity targetEntity;

        Clydesdale::Entity createGravEntity(Clydesdale::AssetManager& assetManager, const Clydesdale::Vector2f position);

    public:
        TestScene(Clydesdale::AssetManager& assetManager, sf::RenderWindow& window);
        ~TestScene();

        void handleEvent(sf::Event event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::Time deltaTime) override;
    };
}
