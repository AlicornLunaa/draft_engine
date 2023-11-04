#include <iostream>
#include <SFML/Graphics.hpp>
#include "util/logger.hpp"
#include "core/application.hpp"

using namespace SpaceGame;

int main(int argc, char** argv){
    // Make sure game can run, handle arguments, etc
    if(!sf::Shader::isAvailable()){
        Util::Logger::println(Util::Logger::Level::CRITICAL, "Space Game", "Shaders unavailable, OpenGL is too old? Cannot continue.");
        return -1;
    }

    // Start game
    Core::Application app("Space Game", 1280, 720);
    app.run();

    return 0;
}