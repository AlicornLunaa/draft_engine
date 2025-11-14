#include "draft/core/application.hpp"
#include "draft/core/scene.hpp"
#include "draft/core/entity.hpp"
#include <tracy/Tracy.hpp>

using namespace std;

namespace Draft {
    Scene::Scene(Application* app) : app(app){}

    Application* Scene::get_app(){
        return app;
    }

    Registry& Scene::get_registry(){
        return registry;
    }

    Entity Scene::create_entity(){
        ZoneScopedN("entity_creation");
        return Entity{ this, registry.create() };
    }

    void Scene::render(Renderer& renderer, Time deltaTime){
        render_world(renderer, deltaTime);
        render_interface(renderer, deltaTime);
    }

    void Scene::handle_event(Event event){}
    void Scene::update(Time timeStep){}
    void Scene::render_world(Renderer& renderer, Time timeStep){}
    void Scene::render_interface(Renderer& renderer, Time timeStep){}
    void Scene::on_attach(){}
    void Scene::on_detach(){}
}