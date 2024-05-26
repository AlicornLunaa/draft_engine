#pragma once

#include "draft/core/registry.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/sprite_batch.hpp"

namespace Draft {
    class RenderSystem {
    private:
        // Variables
        Registry& registryRef;
        RenderWindow& windowRef;

    public:
        // Constructors
        RenderSystem(Registry& registryRef, RenderWindow& windowRef);
        ~RenderSystem();

        // Functions
        void render(SpriteBatch& batch, const Camera* camera = nullptr);
    };
};