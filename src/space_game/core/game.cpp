#include "game.hpp"
#include <SFML/Graphics.hpp>

using namespace SpaceGame;

Game::Game() : Application("Space Game", 1280, 720) {
    // Initialize everything needed for the game
    init();

    // Basic allocations
    const sf::SoundBuffer& soundBuffer = assetManager.getAudio("./assets/audio/boo_womp.mp3");
    sf::Texture& texture1 = assetManager.getTexture("./assets/textures/test_image_1.png");
    texture1.setSmooth(false);

    sprite = sf::Sprite(texture1);
    sound = sf::Sound(soundBuffer);
}

Game::~Game(){}

void Game::handleEvent(){
    switch(event.type){
    case sf::Event::MouseWheelScrolled:
        camera.zoom(1.f + 0.1f * event.mouseWheelScroll.delta);
        break;
    }
}

void Game::init(){
    // Assets
    assetManager.queueTexture("./assets/textures/test_image_1.png");
    assetManager.queueTexture("./assets/textures/test_image_2.jpg");
    assetManager.queueShader("./assets/shaders/default");
    assetManager.queueShader("./assets/shaders/invert");
    assetManager.queueAudio("./assets/audio/boo_womp.mp3");
    assetManager.load();

    // Window settings
    window.setFramerateLimit(60);
}

void Game::draw(){
    window.setView(camera);
    window.draw(sprite);
    world.draw(window);
}