
#include "draft/core/application.hpp"
#include "draft/core/scene.hpp"
#include "draft/core/entity.hpp"

namespace Draft {
    Scene::Scene(Application* app) : app(app){
        // this->uiCamera = sf::View(sf::FloatRect(0, 0, app->window.get_size().x, app->window.get_size().y));
    }

    entt::registry& Scene::get_registry(){
        return registry;
    }

    Entity Scene::create_entity(){
        return { this, registry.create() };
    }

    // void Scene::handleEvent(sf::Event event){
    //     switch(event.type){
    //         case sf::Event::MouseButtonPressed:
    //             Logger::println(Level::INFO, "Scene", std::format("Mouse clicked at x: {:1}, y: {:1}", event.mouseButton.x, event.mouseButton.y));
    //             break;

    //         default:
    //             break;
    //     }
    // }

    void Scene::update(Time deltaTime){
        // TODO: Implementation
    }

    void Scene::render(Time deltaTime){
        // TODO: Implementation
    }
}