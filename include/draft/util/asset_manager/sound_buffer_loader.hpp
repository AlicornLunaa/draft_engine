#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"

namespace Draft {
    struct SoundBufferLoader : public BaseLoader {
        std::vector<std::byte> rawData;

        SoundBufferLoader();

        virtual AssetPtr load_sync() const override;
        virtual void load_async() override;
        virtual AssetPtr finish_async_gl() override;
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override;
    };
};