#include "scene.hpp"
#include "entity.hpp"

namespace Clydesdale {
    Scene::Scene(AssetManager& assetManager, sf::RenderWindow& window){
        this->window = &window;
    }

    entt::registry& Scene::getRegistry(){
        return registry;
    }

    Entity Scene::createEntity(){
        return { this, registry.create() };
    }

    void Scene::handleEvent(sf::Event event){
        // TODO: Implementation
    }

    void Scene::update(sf::Time deltaTime){
        // TODO: Implementation
    }

    void Scene::render(sf::Time deltaTime){
        // TODO: Implementation
    }
}