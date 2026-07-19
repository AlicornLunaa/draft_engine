#include "draft/core/application_interface.hpp"
#include "draft/rendering/camera.hpp"

#include <algorithm>

namespace Draft {
    ApplicationInterface::ApplicationInterface(RenderTarget& target, Keyboard& keyboard, Mouse& mouse) : target(target), keyboard(keyboard), mouse(mouse) {}

    bool ApplicationInterface::dispatch(const Event& event){
        if(eventCallback && eventCallback(event))
            return true;

        if(p_activeScene)
            return p_activeScene->dispatch_event(event);

        return false;
    }

    void ApplicationInterface::scene_change(){
        if(p_activeScene)
            p_activeScene->detach();

        reset_timers();

        p_activeScene = p_newScene;
        p_newScene = nullptr;

        if(p_activeScene)
            p_activeScene->attach();
    }

    void ApplicationInterface::reset_timers(){
        p_accumulator = 0.0;
        p_deltaClock.restart();
        deltaTime = Time();
    }

    void ApplicationInterface::tick(){
        if(simulationPaused){
            p_accumulator = 0.0;
            return;
        }

        p_accumulator += deltaTime.as_seconds();
        p_accumulator = std::min(p_accumulator, (double)maxAccumulator.as_seconds());

        while(p_accumulator >= timeStep.as_seconds()){
            if(p_activeScene)
                p_activeScene->update(timeStep);

            p_accumulator -= timeStep.as_seconds();
        }
    }

    void ApplicationInterface::frame_into(RenderTarget& target){
        target.begin();

        if(p_activeScene){
            // Ordinary per-frame work always runs, even with no renderer set
            p_activeScene->render(deltaTime, RenderLayer::Default);
            
            if(p_renderer){
                OrthographicCamera defaultCamera{Vector3f{0, 0, 10}, Vector3f{0, 0, -1}, 0.f, (float)target.get_size().x, (float)target.get_size().y, 0.f};
                auto* camera = p_activeScene->get_active_camera();

                if(!camera)
                    camera = &defaultCamera;

                p_renderer->render_frame(deltaTime, p_activeScene->get_systems(), *camera);
            }
        }

        target.end();
    }

    void ApplicationInterface::close(){
        // No operation
    }

    void ApplicationInterface::set_scene(Scene* scene){
        p_newScene = scene;
    }

    void ApplicationInterface::set_renderer(std::unique_ptr<Renderer> renderer){
        p_newRenderer = std::move(renderer);
    }
}
