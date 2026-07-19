#include "draft/ecs/scene.hpp"
#include "draft/components/camera_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/ecs/entity.hpp"

#include <limits>

namespace Draft {
    Scene::Scene() : m_relationshipSystem(*this) {}

    Registry& Scene::get_registry(){ return m_registry; }
    const Registry& Scene::get_registry() const { return m_registry; }

    SystemRegistry& Scene::get_systems(){ return m_systems; }
    const SystemRegistry& Scene::get_systems() const { return m_systems; }

    Entity Scene::create_entity(){
        return Entity{ this, m_registry.create() };
    }

    Camera* Scene::get_active_camera(){
        // If an override supplied, just use that
        if(m_cameraOverride){
            return m_cameraOverride.get();
        }

        // Find a camera component
        entt::entity best = entt::null;
        int bestPriority = std::numeric_limits<int>::min();

        for(auto&& [raw, cam] : m_registry.view<CameraComponent>().each()){
            if(!cam.active || !cam.camera)
                continue;

            if(best != entt::null && cam.priority < bestPriority)
                continue;

            best = raw;
            bestPriority = cam.priority;
        }

        if(best == entt::null)
            return nullptr;

        CameraComponent& cam = m_registry.get<CameraComponent>(best);
        if(auto* transform = m_registry.try_get<TransformComponent>(best)){
            cam.camera->set_position({transform->position.x, transform->position.y, 0.f});
            cam.camera->set_rotation(transform->rotation);
        }

        return cam.camera.get();
    }

    void Scene::set_active_camera_override(std::unique_ptr<Camera>&& camera) {
        m_cameraOverride = std::move(camera);
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
