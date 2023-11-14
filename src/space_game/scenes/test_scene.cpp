#include "test_scene.hpp"

using namespace SpaceGame;
using namespace Clyde;

Core::Entity TestScene::createGravEntity(Util::AssetManager& assetManager, const sf::Vector2f position){
    Core::Entity entity = createEntity();
    entity.addComponent<ECS::TransformComponent>(position, 0.f);
    entity.addComponent<ECS::SpriteComponent>(new sf::Sprite(assetManager.getTexture("./assets/textures/test_image_1.png")));
    return entity;
}

TestScene::TestScene(Util::AssetManager& assetManager, sf::RenderWindow& window) : Scene(assetManager, window) {
    camera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    uiCamera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));

    createGravEntity(assetManager, { 0, 0 }).addComponent<ECS::ControlComponent>();
}

void TestScene::handleEvent(sf::Event event){
    Scene::handleEvent(event);
    
    switch(event.type){
    case sf::Event::MouseWheelScrolled:
        camera.zoom(1.f + 0.1f * event.mouseWheelScroll.delta);
        break;
    }
}

void TestScene::update(sf::Time deltaTime){
    Scene::update(deltaTime);

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

    auto view = registry.view<ECS::ControlComponent, ECS::TransformComponent>();
    for(auto entity : view){
        auto& transformComponent = view.get<ECS::TransformComponent>(entity);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
            transformComponent.transform.translate(-100 * deltaTime.asSeconds(), 0);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
            transformComponent.transform.translate(100 * deltaTime.asSeconds(), 0);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
            transformComponent.transform.translate(0, -100 * deltaTime.asSeconds());
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
            transformComponent.transform.translate(0, 100 * deltaTime.asSeconds());
        }
    }
}

void TestScene::render(sf::Time deltaTime){
    Scene::render(deltaTime);
    window->setView(camera);

    auto view = registry.view<ECS::SpriteComponent, ECS::TransformComponent>();
    for(auto entity : view){
        auto& spriteComponent = view.get<ECS::SpriteComponent>(entity);
        auto& transformComponent = view.get<ECS::TransformComponent>(entity);

        sf::RenderStates states = sf::RenderStates::Default;
        states.shader = spriteComponent.shader;
        states.texture = spriteComponent.sprite->getTexture();
        states.transform = transformComponent.transform;

        window->draw(spriteComponent, states);
    }
}
