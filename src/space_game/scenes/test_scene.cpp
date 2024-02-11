#include "test_scene.hpp"
#include "clydesdale/components/transform_component.hpp"
#include "clydesdale/core/entity.hpp"
#include "clydesdale/math/vector2.hpp"
#include <nlohmann/json.hpp>
#include <clydesdale/util/logger.hpp>
#include <string>

using namespace SpaceGame;
using namespace Clyde;
using json = nlohmann::json;

Core::Entity TestScene::createGravEntity(Util::AssetManager& assetManager, const Math::Vector2f position){
    Core::Entity entity = createEntity();
    entity.addComponent<ECS::TransformComponent>(position, 0.f);
    entity.addComponent<ECS::SpriteComponent>(sprite);
    return entity;
}

TestScene::TestScene(Util::AssetManager& assetManager, sf::RenderWindow& window) : Scene(assetManager, window) {
    uiCamera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    sprite = new sf::Sprite(assetManager.getTexture("./assets/textures/test_image_1.png"));

    camera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    camera.setCenter(0, 0);

    Phys::BodyDef groundBodyDef;
    Phys::PolygonShape groundBox;
    groundBodyDef.position.Set(0.0f, -100.0f);
    groundBox.SetAsBox(50.0f, 10.0f);
    ground = world.createBody(groundBodyDef);
    ground.createFixture(&groundBox, 0.f);

    Phys::BodyDef bodyDef;
    Phys::FixtureDef fixtureDef;
    Phys::PolygonShape dynamicBox;
    bodyDef.type = Phys::BodyType::b2_dynamicBody;
    bodyDef.position.Set(0.0f, 4.0f);
    bodyDef.angle = 1;
    dynamicBox.SetAsBox(1.0f, 1.0f);
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body = world.createBody(bodyDef);
    body.createFixture(&fixtureDef);

    createGravEntity(assetManager, { 0, 0 }).addComponent<ECS::ControlComponent>();
    targetEntity = createGravEntity(assetManager, { -200, 4.f });

    console.registerCmd("set_pos", [this](){
        auto& transform = targetEntity.getComponent<ECS::TransformComponent>();
        transform.transform.translate(128.f, 0);
        console.print("Test");
    });

    console.registerCmd("test_cmd", [](){
        Util::Logger::println(Util::Logger::Level::INFO, "Console", "Hello world!");
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

    // auto view = registry.view<ECS::TransformComponent>();
    // for(auto entity : view){
    //     auto& transformComponent = view.get<ECS::TransformComponent>(entity);

    //     Clyde::Math::Transform trans;
    //     // trans.rotate(body.getAngle());
    //     // trans.translate(body.getPosition());
    //     transformComponent.transform = trans;
    // }
}

void TestScene::render(sf::Time deltaTime){
    Scene::render(deltaTime);
    console.draw();

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
