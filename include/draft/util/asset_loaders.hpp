#pragma once

#include "draft/rendering/particle_system.hpp"
#include "draft/util/asset_manager.hpp"
#include "nlohmann/json_fwd.hpp"

#include <string>

namespace Draft {
    struct ParticleLoader : public Assets::BaseLoader {
        ParticleProps* propPtr = nullptr;
        std::string texture;

        virtual std::shared_ptr<void> load_sync() const override;
        virtual void load_async() override;
        virtual std::shared_ptr<void> finish_async_gl() override;
        virtual BaseLoader* clone(const FileHandle& handle) const override;
    };

    struct JSONLoader : public Assets::BaseLoader {
        nlohmann::json* data;
        
        virtual std::shared_ptr<void> load_sync() const override;
        virtual void load_async() override;
        virtual std::shared_ptr<void> finish_async_gl() override;
        virtual BaseLoader* clone(const FileHandle& handle) const override;
    };
}