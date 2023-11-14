#include <iostream>
#include <space_game/core/game.hpp>

int main(int argc, char** argv){
    // Start game
    // SpaceGame::Game game;
    // game.run();

    Clyde::Math::Vector2f v = Clyde::Math::Vector2f(10, 25);
    Clyde::Math::Vector2f x = Clyde::Math::Vector2f(40, 40);
    std::cout << v.x << ", " << v.y << "\n";
    v += x;
    std::cout << v.x << ", " << v.y << "\n";

    return 0;
}