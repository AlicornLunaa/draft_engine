#pragma once

#include "draft/phys/collider.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"

namespace Draft {
    struct ColliderLoader : public BaseLoader {
        Collider* collider = nullptr;

        ColliderLoader();
        
        virtual AssetPtr load_sync(Assets& assets) const override;
        virtual void load_async() override;
        virtual AssetPtr finish_async_gl(Assets& assets) override;
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override;
    };
};