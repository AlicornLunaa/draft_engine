#pragma once

#include "draft/interface/widgets/panel.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    namespace UI {
        class Picture : public Panel {
        public:
            // Variables
            TextureRegion region;
            Vector4f tint{1};

            // Constructor
            Picture(SNumber x, SNumber y, SNumber w, SNumber h, Resource<Texture> texture, Panel* parent = nullptr);

            // Functions
            virtual void paint(Context& ctx) override;
        };
    }
};