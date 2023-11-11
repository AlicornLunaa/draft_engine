#pragma once
#include <clydesdale_engine/core.hpp>
#include <clydesdale_engine/ecs.hpp>

namespace SpaceGame {
    class Game : public Clydesdale::Core::Application {
    private:
        Clydesdale::Core::Scene scene1;

    public:
        Game();
    };
}