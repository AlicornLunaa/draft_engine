#pragma once

#include "draft/interface/panel.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    namespace UI {
        class Picture : public Panel {
        private:
            // Vars
            Resource<Texture> texture;

        public:
            // Constructor
            Picture(float x, float y, float w, float h, Resource<Texture> texture, Panel* parent = nullptr);

            // Functions
            virtual void paint(const Time& deltaTime, SpriteBatch& batch) override;
        };
    }
};