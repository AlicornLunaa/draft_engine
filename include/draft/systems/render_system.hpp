#pragma once

#include "draft/core/registry.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"

namespace Draft {
    class RenderSystem {
    private:
        // Variables
        Registry& registryRef;
        RenderWindow& windowRef;

    public:
        // Constructors
        RenderSystem(Registry& registryRef, RenderWindow& windowRef);
        RenderSystem(const RenderSystem& other) = delete;
        ~RenderSystem();

        // Operators
        RenderSystem& operator=(const RenderSystem& other) = delete;

        // Functions
        void render(SpriteBatch& batch);
    };
};