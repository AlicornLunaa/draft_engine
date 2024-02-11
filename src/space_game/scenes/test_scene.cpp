#include "test_scene.hpp"
#include <clydesdale/engine.hpp>
#include <nlohmann/json.hpp>
#include <string>

using namespace SpaceGame;
using namespace Clydesdale;
using json = nlohmann::json;

Entity TestScene::createGravEntity(AssetManager& assetManager, const Vector2f position){
    Entity entity = createEntity();
    entity.addComponent<TransformComponent>(position, 0.f);
    entity.addComponent<SpriteComponent>(sprite);
    return entity;
}

TestScene::TestScene(AssetManager& assetManager, sf::RenderWindow& window) : Scene(assetManager, window) {
    uiCamera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    sprite = new sf::Sprite(assetManager.getTexture("./assets/textures/test_image_1.png"));

    camera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    camera.setCenter(0, 0);

    BodyDef groundBodyDef;
    PolygonShape groundBox;
    groundBodyDef.position.Set(0.0f, -100.0f);
    groundBox.SetAsBox(50.0f, 10.0f);
    ground = world.createBody(groundBodyDef);
    ground.createFixture(&groundBox, 0.f);

    BodyDef bodyDef;
    FixtureDef fixtureDef;
    PolygonShape dynamicBox;
    bodyDef.type = BodyType::b2_dynamicBody;
    bodyDef.position.Set(0.0f, 4.0f);
    bodyDef.angle = 1;
    dynamicBox.SetAsBox(1.0f, 1.0f);
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body = world.createBody(bodyDef);
    body.createFixture(&fixtureDef);

    createGravEntity(assetManager, { 0, 0 }).addComponent<ControlComponent>();
    targetEntity = createGravEntity(assetManager, { -200, 4.f });

    console.registerCmd("set_pos", [this](){
        auto& transform = targetEntity.getComponent<TransformComponent>();
        transform.transform.translate(128.f, 0);
        console.print("Test");
    });

    console.registerCmd("test_cmd", [](){
        Logger::println(Level::INFO, "Console", "Hello world!");
    });

    console.registerCmd("quit", [this](){
        this->window->close();
    });
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

    if(!console.isOpened()){
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)) camera.move(-100 * deltaTime.asSeconds(), 0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)) camera.move(100 * deltaTime.asSeconds(), 0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)) camera.move(0, -100 * deltaTime.asSeconds());
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)) camera.move(0, 100 * deltaTime.asSeconds());
    }

    world.step(TIME_STEP, VELOCITY_ITER, POSITION_ITER);

    // auto view = registry.view<TransformComponent>();
    // for(auto entity : view){
    //     auto& transformComponent = view.get<TransformComponent>(entity);

    //     Clyde::Transform trans;
    //     // trans.rotate(body.getAngle());
    //     // trans.translate(body.getPosition());
    //     transformComponent.transform = trans;
    // }
}

void TestScene::render(sf::Time deltaTime){
    Scene::render(deltaTime);
    console.draw();

    window->setView(camera);

    auto view = registry.view<SpriteComponent, TransformComponent>();
    for(auto entity : view){
        auto& spriteComponent = view.get<SpriteComponent>(entity);
        auto& transformComponent = view.get<TransformComponent>(entity);

        sf::RenderStates states = sf::RenderStates::Default;
        states.shader = spriteComponent.shader;
        states.texture = spriteComponent.sprite->getTexture();
        states.transform = transformComponent;

        window->draw(spriteComponent, states);
    }
}
