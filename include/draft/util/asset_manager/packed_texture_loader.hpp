#pragma once

#include "draft/rendering/texture_packer.hpp"
#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"

namespace Draft {
    struct PackedTextureLoader : public BaseLoader {
        TexturePacker* packer;

        PackedTextureLoader();

        virtual AssetPtr load_sync() const override;
        virtual void load_async() override;
        virtual AssetPtr finish_async_gl() override;
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override;
    };
};