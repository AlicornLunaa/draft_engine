#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/core/engine.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Draft {
    /**
     * @brief The asset types draft_buildtools knows how to validate/pack, by extension (see classify_asset()).
     */
    enum class AssetKind {
        Texture,
        Font,
        Model,
        Sound,
        Music,
        Scene,
        Prefab,
        RML,
        RCSS,
        Animation,
        Language,
        Particle,
        Shader,
        Unknown
    };

    const char* asset_kind_name(AssetKind kind);

    /**
     * @brief Classifies @p projectRelativePath by extension alone
     */
    AssetKind classify_asset(const std::filesystem::path& projectRelativePath);

    struct AssetTask {
        std::string key; // project-root-relative, forward slashes, e.g. "assets/textures/dev.png"
        AssetKind kind;
    };

    /**
     * @brief Walks <projectRoot>/assets recursively. A directory directly containing both
     * vertex.glsl and fragment.glsl is reported once as a Shader task (keyed by the directory
     * itself, not either file inside it); everything else is classified per file, skipping
     * anything classify_asset() reports as Unknown (READMEs, loose .glsl includes, ...).
     */
    std::vector<AssetTask> collect_project_assets(const std::filesystem::path& projectRoot);

    /**
     * @brief Registers Texture/Font/Model/SoundBuffer default loaders on @p assets, queues and
     * synchronously loads every non-Scene task, and validates every Scene task by calling
     * load_scene() against a scratch Scene using @p sceneEngine's catalogs (load_scene() doesn't
     * go through AssetManager's loader registry, so it can't be queued the same way).
     *
     * Requires a live GL context as Texture/Font/Model construction all issue real GL calls.
     *
     * @return Failed task key -> error message. Empty means every task validated.
     */
    std::unordered_map<std::string, std::string> validate_assets(AssetManager& assets, Engine& sceneEngine, const std::vector<AssetTask>& tasks);
}
