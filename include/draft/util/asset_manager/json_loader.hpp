#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/json.hpp"

namespace Draft {
    struct JSONLoader : public BaseLoader {
        JSON* data;

        JSONLoader();
        
        virtual AssetPtr load_sync(Assets& assets) const override;
        virtual void load_async() override;
        virtual AssetPtr finish_async_gl(Assets& assets) override;
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override;
    };
};