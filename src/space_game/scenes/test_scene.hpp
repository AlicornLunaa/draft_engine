#pragma once
#include <clydesdale/engine.hpp>
#include <box2d/box2d.h>

namespace SpaceGame {
    class TestScene : public Clyde::Core::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;
        sf::Sprite* sprite;

        Clyde::Phys::World world = Clyde::Phys::World(Clyde::Math::Vector2f(0, -10));
        Clyde::Phys::RigidBody* ground;
        Clyde::Phys::RigidBody* body;

        Clyde::Core::Entity createGravEntity(Clyde::Util::AssetManager& assetManager, const Clyde::Math::Vector2f position);

    public:
        TestScene(Clyde::Util::AssetManager& assetManager, sf::RenderWindow& window);
        ~TestScene();

        void handleEvent(sf::Event event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::Time deltaTime) override;
    };
}
