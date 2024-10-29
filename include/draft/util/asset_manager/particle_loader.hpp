#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/rendering/particle_system.hpp"

namespace Draft {
    struct ParticleLoader : public BaseLoader {
        ParticleProps* propPtr = nullptr;
        std::string texture;

        ParticleLoader();

        virtual AssetPtr load_sync() const override;
        virtual void load_async() override;
        virtual AssetPtr finish_async_gl() override;
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override;
    };
};