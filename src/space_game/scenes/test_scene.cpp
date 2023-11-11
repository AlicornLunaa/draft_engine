#include "test_scene.hpp"
#include <clydesdale_engine/ecs.hpp>
#include <clydesdale_engine/util/logger.hpp>
using namespace SpaceGame;

TestScene::TestScene(Clydesdale::Util::AssetManager& assetManager, sf::RenderWindow& window) : Scene(assetManager, window) {
    camera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    uiCamera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));

    sf::Sprite sprite = sf::Sprite(assetManager.getTexture("./assets/test_image_1.png"));
    Clydesdale::Core::Entity entity = createEntity();
    entity.addComponent<Clydesdale::ECS::TransformComponent>();
    entity.addComponent<Clydesdale::ECS::SpriteComponent>(&sprite);
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
}

void TestScene::render(sf::Time deltaTime){
    Scene::render(deltaTime);
    
    window->setView(camera);
    Clydesdale::Util::Logger::printRaw("Hello!\n");

    auto view = registry.view<Clydesdale::ECS::SpriteComponent, Clydesdale::ECS::TransformComponent>();
    for(auto entity : view){
        auto& spriteComponent = view.get<Clydesdale::ECS::SpriteComponent>(entity);
        auto& transform = view.get<Clydesdale::ECS::TransformComponent>(entity);

        spriteComponent.sprite->setPosition(transform.x, transform.y);
        
        if(spriteComponent.shader){
            window->draw(spriteComponent, spriteComponent.shader);
        } else {
            window->draw(spriteComponent);
        }
    }
}
