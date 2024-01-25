#include "test_scene.hpp"

using namespace SpaceGame;
using namespace Clyde;

Core::Entity TestScene::createGravEntity(Util::AssetManager& assetManager, const Math::Vector2f position){
    Core::Entity entity = createEntity();
    entity.addComponent<ECS::TransformComponent>(position, 0.f);
    entity.addComponent<ECS::SpriteComponent>(sprite);
    return entity;
}

TestScene::TestScene(Util::AssetManager& assetManager, sf::RenderWindow& window) : Scene(assetManager, window) {
    camera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    uiCamera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    sprite = new sf::Sprite(assetManager.getTexture("./assets/textures/test_image_1.png"));

    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -100.0f);
    ground = world.createBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 10.0f);
    ground->createFixture(&groundBox, 0.f);

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0f, 4.0f);
    body = world.createBody(&bodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(1.0f, 1.0f);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body->createFixture(&fixtureDef);

    createGravEntity(assetManager, { 0, 0 }).addComponent<ECS::ControlComponent>();
}
TestScene::~TestScene(){
    delete sprite;
}

void TestScene::handleEvent(sf::Event event){
    Scene::handleEvent(event);
    
    switch(event.type){
    case sf::Event::MouseWheelScrolled:
        camera.zoom(1.f + 0.1f * event.mouseWheelScroll.delta);
        break;
    default:
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

    world.step(TIME_STEP, VELOCITY_ITER, POSITION_ITER);

    auto view = registry.view<ECS::ControlComponent, ECS::TransformComponent>();
    for(auto entity : view){
        auto& transformComponent = view.get<ECS::TransformComponent>(entity);

        Clyde::Math::Transform trans;
        trans.rotate(body->getAngle());
        trans.translate(Clyde::Math::Vector2f(body->getPosition()));
        transformComponent.transform = trans;
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
        states.transform = transformComponent;

        window->draw(spriteComponent, states);
    }
}
