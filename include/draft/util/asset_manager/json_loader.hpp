#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "nlohmann/json_fwd.hpp"

namespace Draft {
    struct JSONLoader : public BaseLoader {
        nlohmann::json* data;

        JSONLoader();
        
        virtual AssetPtr load_sync() const override;
        virtual void load_async() override;
        virtual AssetPtr finish_async_gl() override;
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override;
    };
};