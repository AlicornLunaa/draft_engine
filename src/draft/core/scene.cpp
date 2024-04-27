#include <string>

#include "draft/core/application.hpp"
#include "draft/core/scene.hpp"
#include "draft/core/entity.hpp"
#include "draft/util/logger.hpp"

using namespace std;

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

    void Scene::handleEvent(Event event){
        switch(event.type){
            case Event::MouseButtonPressed:
                Logger::println(Level::INFO, "Scene", "Mouse clicked at x: " + to_string(event.mouseButton.x) + ", y: " + to_string(event.mouseButton.y));
                break;

            default:
                break;
        }
    }

    void Scene::update(Time deltaTime){
        // TODO: Implementation
    }

    void Scene::render(Time deltaTime){
        // TODO: Implementation
    }
}