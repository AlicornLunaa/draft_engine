#pragma once

#include "draft/rendering/image.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"

namespace Draft {
    struct ImageLoader : public BaseLoader {
        Image* imgPtr = nullptr;

        ImageLoader();

        virtual AssetPtr load_sync() const override;
        virtual void load_async() override;
        virtual AssetPtr finish_async_gl() override;
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override;
    };
};