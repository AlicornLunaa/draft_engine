#include "test_scene.hpp"
#include "clydesdale/components/rigid_body_component.hpp"
#include "clydesdale/core/entity.hpp"
#include "clydesdale/util/constants.hpp"
#include <clydesdale/engine.hpp>
#include <nlohmann/json.hpp>
#include <string>

using namespace Clydesdale;
using json = nlohmann::json;

namespace SpaceGame {
    Entity TestScene::createGravEntity(AssetManager& assetManager, const Vector2f position){
        Entity entity = createEntity();
        entity.addComponent<TransformComponent>(position, 0.f);
        entity.addComponent<SpriteComponent>(sprite1);
        return entity;
    }

    Entity TestScene::createGroundEntity(AssetManager& assetManager, const Vector2f position, const Vector2f size){
        BodyDef groundBodyDef;
        groundBodyDef.position.Set(position.x, position.y);

        // const auto& textureSize = sprite2->getTexture()->getSize();
        // sprite2->setScale(size.x / textureSize.x, size.y / textureSize.y);
        // sprite2->setPosition(textureSize.x / -2.f, textureSize.y / -2.f);

        Entity entity = createEntity();
        entity.addComponent<TransformComponent>(position, 0.f);
        entity.addComponent<SpriteComponent>(sprite2);
        entity.addComponent<RigidBodyComponent>(world, groundBodyDef, size.x, size.y);
        return entity;
    }

    TestScene::TestScene(AssetManager& assetManager, sf::RenderWindow& window) : Scene(assetManager, window) {
        uiCamera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
        sprite1 = new sf::Sprite(assetManager.getTexture("./assets/textures/test_image_1.png"));
        sprite2 = new sf::Sprite(assetManager.getTexture("./assets/textures/test_image_3.png"));

        sprite1->setPosition(-64, -64);
        sprite2->setPosition(-64, -64);

        camera = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
        camera.setCenter(0, 0);

        BodyDef bodyDef;
        bodyDef.type = BodyType::b2_dynamicBody;
        bodyDef.position.Set(0.0f, 0.0f);
        bodyDef.angle = TO_RAD(5);

        // createGroundEntity(assetManager, { 0, -100 }, { 50.f, 10.f });
        createGroundEntity(assetManager, { 0, -256 }, { 128.f, 128.f });
        createGravEntity(assetManager, { 0, 0 }).addComponent<RigidBodyComponent>(world, bodyDef, 128.f, 128.f);
        targetEntity = createGravEntity(assetManager, { -200, 4.f });

        console.registerCmd("set_pos", [this](ConsoleArgs args){
            if(args.size() < 3){
                console.print("Usage: " + args[0] + " x y");
                return false;
            }
            
            float x = stof(args[1]);
            float y = stof(args[2]);
            
            auto& transform = targetEntity.getComponent<TransformComponent>();
            transform.transform.translate(x, y);
            return true;
        });

        console.registerCmd("test_cmd", [this](ConsoleArgs args){
            for(const std::string& s : args){
                console.print(s);
            }

            Logger::println(Level::INFO, "Console", "Hello world!");
            return true;
        });

        console.registerCmd("quit", [this](ConsoleArgs args){
            this->window->close();
            return true;
        });
    }
    TestScene::~TestScene(){
        delete sprite1;
        delete sprite2;
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

        auto view = registry.view<TransformComponent, RigidBodyComponent>();
        for(auto entity : view){
            auto& transformComponent = view.get<TransformComponent>(entity);
            auto& rigidBodyComponent = view.get<RigidBodyComponent>(entity);

            Transform trans;
            trans.translate(rigidBodyComponent.getPosition());
            trans.rotate(TO_DEG(rigidBodyComponent.getAngle()));
            transformComponent.transform = trans;
        }
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
}