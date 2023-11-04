#include <iostream>
#include <SFML/Graphics.hpp>
#include <space_game/core/game.hpp>
#include <clydesdale_engine/util/logger.hpp>

using namespace Clydesdale;

int main(int argc, char** argv){
    // Start game
    SpaceGame::Game game;
    game.run();

    return 0;
}