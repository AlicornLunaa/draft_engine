#include "scene.hpp"
#include "entity.hpp"
using namespace Clydesdale::Core;

Scene::Scene(sf::RenderWindow& window){
    this->window = &window;
}

entt::registry& Scene::getRegistry(){
    return registry;
}

Entity Scene::createEntity(){
    return { this, registry.create() };
}

void Scene::update(sf::Time deltaTime){
    
}

void Scene::render(sf::Time deltaTime){
    
}