#pragma once
#include <clydesdale_engine/core.hpp>
#include <clydesdale_engine/ecs.hpp>
#include <space_game/scenes/test_scene.hpp>

namespace SpaceGame {
    class Game : public Clydesdale::Core::Application {
    private:
        TestScene* scene1;

    public:
        Game();
        ~Game();
    };
}