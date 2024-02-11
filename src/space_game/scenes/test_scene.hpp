#pragma once
#include <clydesdale/engine.hpp>

namespace SpaceGame {
    class TestScene : public Clyde::Core::Scene {
    private:
        sf::View uiCamera;
        sf::View camera;
        sf::Sprite* sprite;

        Cl::Widgets::Console console;

        Cl::Phys::World world = Cl::Phys::World(Cl::Math::Vector2f(0, 0));
        Cl::Phys::RigidBody ground;
        Cl::Phys::RigidBody body;
        Cl::Core::Entity targetEntity;

        Cl::Core::Entity createGravEntity(Cl::Util::AssetManager& assetManager, const Cl::Math::Vector2f position);

    public:
        TestScene(Cl::Util::AssetManager& assetManager, sf::RenderWindow& window);
        ~TestScene();

        void handleEvent(sf::Event event) override;
        void update(sf::Time deltaTime) override;
        void render(sf::Time deltaTime) override;
    };
}
