#pragma once
#include <clydesdale_engine/core/application.hpp>
#include <clydesdale_engine/ecs.hpp>

namespace SpaceGame {
    class Game : public Clydesdale::Core::Application {
    private:
        sf::View uiCamera = sf::View(sf::FloatRect(0, 0, 1280, 720));
        sf::View camera = sf::View(sf::FloatRect(0, 0, 1280, 720));
        
        entt::registry mRegistry;

        sf::Texture* texture1;
        sf::Texture* texture2;
        sf::Shader* shader;
        sf::Sprite sprite;
        sf::Sound sound;
        SpaceGame::Simulation::World world = SpaceGame::Simulation::World(50, 50);

    public:
        Game();
        ~Game();

        void handleEvent();
        void init();
        void draw();
    };
}