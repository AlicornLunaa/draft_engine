#pragma once

#include "draft/interface/panel.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager/resource.hpp"

namespace Draft {
    namespace UI {
        class Picture : public Panel {
        private:
            Resource<Texture> texture;
            void update_img();

        public:
            Picture(float x, float y, float w, float h, Resource<Texture> texture);
        };
    }
};