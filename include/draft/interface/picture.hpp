#pragma once

#include "draft/interface/panel.hpp"
#include "draft/rendering/texture.hpp"

namespace Draft {
    namespace UI {
        class Picture : public Panel {
        private:
            const Texture& texture;
            void update_img();

        public:
            Picture(float x, float y, float w, float h, const Texture& texture);
        };
    }
};