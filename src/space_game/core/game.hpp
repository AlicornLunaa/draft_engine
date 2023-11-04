#include <clydesdale_engine/core/application.hpp>

namespace SpaceGame {
    class Game : public Clydesdale::Core::Application {
    private:
        sf::View uiCamera = sf::View(sf::FloatRect(0, 0, 1280, 720));
        sf::View camera = sf::View(sf::FloatRect(0, 0, 640, 480));

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