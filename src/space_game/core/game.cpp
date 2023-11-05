#include "game.hpp"
#include <SFML/Graphics.hpp>

using namespace SpaceGame;

Game::Game() : Application("Space Game", 1280, 720) {
    // Initialize everything needed for the game
    init();

    // Basic allocations
    const sf::SoundBuffer& soundBuffer = assetManager.getAudio("./assets/audio/boo_womp.mp3");
    texture1 = &assetManager.getTexture("./assets/textures/test_image_1.png");
    texture2 = &assetManager.getTexture("./assets/textures/test_image_3.png");
    texture1->setSmooth(false);

    shader = &assetManager.getShader("./assets/shaders/default");
    sprite = sf::Sprite(*texture1);
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
    assetManager.queueTexture("./assets/textures/test_image_3.png");
    assetManager.queueShader("./assets/shaders/default");
    assetManager.queueShader("./assets/shaders/invert");
    assetManager.queueAudio("./assets/audio/boo_womp.mp3");
    assetManager.queueFont("./assets/fonts/default.ttf");
    assetManager.load();

    // Window settings
    window.setFramerateLimit(60);
}

void Game::draw(){
    window.setView(camera);

    sprite.setPosition(1280/2 - 64, 720/2 - 64);
    shader->setUniform("texture1", *texture1);
    shader->setUniform("texture2", *texture2);
    window.draw(sprite, shader);

    // world.draw(window);
}