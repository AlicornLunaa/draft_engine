#include "game.hpp"
#include <SFML/Graphics.hpp>

using namespace SpaceGame;
using namespace Clydesdale;

Game::Game() : Application("Space Game", 1280, 720), scene(window) {
    // Initialize everything needed for the game
    init();

    // Basic allocations
    const sf::SoundBuffer& soundBuffer = assetManager.getAudio("./assets/audio/boo_womp.mp3");
    texture1 = &assetManager.getTexture("./assets/textures/test_image_1.png");
    texture2 = &assetManager.getTexture("./assets/textures/test_image_3.png");
    texture1->setSmooth(false);

    shader = &assetManager.getShader("./assets/shaders/lighting_specular");
    sprite = sf::Sprite(*texture1);
    sound = sf::Sound(soundBuffer);
    sprite.setPosition(1280/2 - 256, 720/2 - 64);
    
    entity = scene.createEntity();
    entity.addComponent<ECS::TransformComponent>();
    entity.addComponent<ECS::SpriteComponent>(&sprite);
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
    assetManager.queueShader("./assets/shaders/lighting_specular");
    assetManager.queueAudio("./assets/audio/boo_womp.mp3");
    assetManager.queueFont("./assets/fonts/default.ttf");
    assetManager.load();

    // Window settings
    window.setFramerateLimit(60);
}

void Game::draw(){
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
        camera.move(-100 * deltaTime.asSeconds(), 0);
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
        camera.move(100 * deltaTime.asSeconds(), 0);
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
        camera.move(0, -100 * deltaTime.asSeconds());
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
        camera.move(0, 100 * deltaTime.asSeconds());
    }

    {
        auto& transform = entity.getComponent<ECS::TransformComponent>();
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) transform.x += -100 * deltaTime.asSeconds();
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) transform.x += 100 * deltaTime.asSeconds();
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) transform.y += 100 * deltaTime.asSeconds();
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) transform.y += -100 * deltaTime.asSeconds();
    }
    
    window.setView(camera);

    auto view = scene.getRegistry().view<ECS::SpriteComponent, ECS::TransformComponent>();
    for(auto entity : view){
        auto& spriteComponent = view.get<ECS::SpriteComponent>(entity);
        auto& transform = view.get<ECS::TransformComponent>(entity);

        spriteComponent.sprite->setPosition(transform.x, transform.y);
        
        if(spriteComponent.shader){
            window.draw(spriteComponent, spriteComponent.shader);
        } else {
            window.draw(spriteComponent);
        }
    }
}