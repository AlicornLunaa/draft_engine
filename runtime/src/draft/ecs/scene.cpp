#include "draft/ecs/scene.hpp"
#include "draft/ecs/entity.hpp"

namespace Draft {
    Scene::Scene() : m_relationshipSystem(*this) {}

    Registry& Scene::get_registry(){ return m_registry; }
    const Registry& Scene::get_registry() const { return m_registry; }

    SystemRegistry& Scene::get_systems(){ return m_systems; }
    const SystemRegistry& Scene::get_systems() const { return m_systems; }

    Entity Scene::create_entity(){
        return Entity{ this, m_registry.create() };
    }

    void Scene::update(Time dt){
        m_systems.update_all(dt);
    }

    void Scene::render(Time dt, RenderLayer layer){
        m_systems.render_all(dt, layer);
    }

    void Scene::attach(){
        m_systems.attach_all();
    }

    void Scene::detach(){
        m_systems.detach_all();
    }

    bool Scene::dispatch_event(const Event& event){
        return m_systems.dispatch_event(event);
    }
}
