#pragma once

#include "draft/asset/asset_manager.hpp"

namespace Draft {
    namespace Loaders {
        template<typename T>
        void register_default_loader(AssetManager& assets);
    }
}