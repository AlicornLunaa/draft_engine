#pragma once

#include "draft/util/asset_manager/base_loader.hpp"
#include "nlohmann/json_fwd.hpp"

namespace Draft {
    struct JSONLoader : public BaseLoader {
        nlohmann::json* data;
        
        virtual std::shared_ptr<void> load_sync() const override;
        virtual void load_async() override;
        virtual std::shared_ptr<void> finish_async_gl() override;
        virtual BaseLoader* clone(const FileHandle& handle) const override;
    };
};