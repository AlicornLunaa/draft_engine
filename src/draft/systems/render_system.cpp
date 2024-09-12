#include "draft/rendering/camera.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/components/sprite_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/systems/render_system.hpp"

namespace Draft {
    // Constructors
    RenderSystem::RenderSystem(Registry& registryRef, RenderWindow& windowRef) : registryRef(registryRef), windowRef(windowRef) {
        // Attach listeners
    }

    RenderSystem::~RenderSystem(){
    }

    // Functions
    void RenderSystem::render(SpriteBatch& batch, const Camera* camera){
        auto view = registryRef.view<SpriteComponent, TransformComponent>();

        batch.set_proj_matrix(camera->get_combined());
        batch.begin();

        for(auto entity : view){
            auto& spriteComponent = view.get<SpriteComponent>(entity);
            auto& transformComponent = view.get<TransformComponent>(entity);

            batch.draw({
                spriteComponent.texture,
                transformComponent.position,
                transformComponent.rotation,
                spriteComponent.size,
                spriteComponent.origin,
                spriteComponent.zIndex
            });
        }

        batch.flush();
    }
};