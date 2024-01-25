#pragma once
#include <clydesdale/engine.hpp>
#include <box2d/box2d.h>

namespace SpaceGame {
    class TestScene : public Clyde::Core::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;
        sf::Sprite* sprite;

        b2Vec2 grav = b2Vec2(0, -10);
        b2World world = b2World(grav);
        // Clyde::Phys::RigidBody* ground;
        // Clyde::Phys::RigidBody* body;
        b2Body* ground;
        b2Body* body;

        Clyde::Core::Entity createGravEntity(Clyde::Util::AssetManager& assetManager, const Clyde::Math::Vector2f position);

    public:
        TestScene(Clyde::Util::AssetManager& assetManager, sf::RenderWindow& window);
        ~TestScene();

        void handleEvent(sf::Event event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::Time deltaTime) override;
    };
}
