#include "game.hpp"

using namespace SpaceGame;
using namespace Clydesdale;

Game::Game() : Application("Space Game", 1280, 720) {
    // Assets
    assetManager.queueTexture("./assets/textures/test_image_1.png");
    assetManager.queueTexture("./assets/textures/test_image_2.jpg");
    assetManager.queueTexture("./assets/textures/test_image_3.png");
    assetManager.queueShader("./assets/shaders/default");
    assetManager.queueShader("./assets/shaders/invert");
    assetManager.queueShader("./assets/shaders/lighting_specular");
    assetManager.queueAudio("./assets/audio/boo_womp.mp3");
    assetManager.queueFont("./assets/fonts/default.ttf");
    assetManager.load();

    // Window settings
    window.setFramerateLimit(60);
    
    // Start first scene
    scene1 = new TestScene(assetManager, window);
    this->setScene(scene1);
}

Game::~Game(){
    delete scene1;
}