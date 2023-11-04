#include <iostream>
#include <SFML/Graphics.hpp>
#include "util/logger.hpp"
#include "core/application.hpp"

using namespace Krypton;

int main(int argc, char** argv){
    // Make sure game can run, handle arguments, etc
    if(!sf::Shader::isAvailable()){
        Util::Logger::println(Util::Logger::Level::CRITICAL, "Krypton", "Shaders unavailable, OpenGL is too old? Cannot continue.");
        return -1;
    }

    // Start game
    Core::Application app("Krypton", 1280, 720);
    app.run();

    return 0;
}