#include "draft/core/application.hpp"
#include "draft/core/scene.hpp"
#include "draft/core/entity.hpp"

using namespace std;

namespace Draft {
    Scene::Scene(Application* app) : app(app){}

    Registry& Scene::get_registry(){
        return registry;
    }

    Entity Scene::create_entity(){
        return Entity{ this, registry.create() };
    }

    void Scene::handleEvent(Event event){}
    void Scene::update(Time deltaTime){}
    void Scene::render(Time deltaTime){}
}