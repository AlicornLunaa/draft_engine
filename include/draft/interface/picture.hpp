#pragma once

#include "draft/interface/widgets/panel.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    namespace UI {
        class Picture : public Panel {
        public:
            // Variables
            TextureRegion region;

            // Constructor
            Picture(float x, float y, float w, float h, Resource<Texture> texture, Panel* parent = nullptr);

            // Functions
            virtual void paint(Context& ctx) override;
        };
    }
};