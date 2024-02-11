#pragma once
#include <clydesdale/engine.hpp>
#include <space_game/scenes/test_scene.hpp>

namespace SpaceGame {
    class Game : public Clydesdale::Application {
    private:
        TestScene* scene1;

    public:
        Game();
        ~Game();
    };
}