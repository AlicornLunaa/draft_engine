#pragma once

#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/rendering/particle_system.hpp"

namespace Draft {
    struct ParticleLoader : public BaseLoader {
        ParticleProps* propPtr = nullptr;
        std::string texture;

        virtual std::shared_ptr<void> load_sync() const override;
        virtual void load_async() override;
        virtual std::shared_ptr<void> finish_async_gl() override;
        virtual BaseLoader* clone(const FileHandle& handle) const override;
    };
};