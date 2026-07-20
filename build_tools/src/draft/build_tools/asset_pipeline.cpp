#include "draft/build_tools/asset_pipeline.hpp"
#include "draft/asset/default_loaders.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/ecs/scene_serializer.hpp"
#include "draft/rendering/animation.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/files/host_file_system.hpp"

#include <algorithm>

namespace fs = std::filesystem;

namespace {
    std::string describe(std::exception_ptr error) {
        try {
            if (error)
                std::rethrow_exception(error);
        } catch (const std::exception& e) {
            return e.what();
        } catch (...) {}

        return "unknown error";
    }
}

namespace Draft {
    const char* asset_kind_name(AssetKind kind) {
        switch (kind) {
            case AssetKind::Texture: return "Texture";
            case AssetKind::Font: return "Font";
            case AssetKind::Model: return "Model";
            case AssetKind::Sound: return "Sound";
            case AssetKind::Scene: return "Scene";
            case AssetKind::Prefab: return "Prefab";
            case AssetKind::RML: return "Rml Document";
            case AssetKind::RCSS: return "Rml Stylesheet";
            case AssetKind::Animation: return "Animation";
            case AssetKind::Language: return "Language";
            default: return "Unknown";
        }
    }

    AssetKind classify_asset(const fs::path& projectRelativePath) {
        std::string ext = projectRelativePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") return AssetKind::Texture;
        if (ext == ".ttf") return AssetKind::Font;
        if (ext == ".glb" || ext == ".gltf") return AssetKind::Model;
        if (ext == ".wav" || ext == ".ogg") return AssetKind::Sound;
        if (ext == ".rml") return AssetKind::RML;
        if (ext == ".rcss") return AssetKind::RCSS;
        if (ext == ".anim") return AssetKind::Animation;
        if (ext == ".lang") return AssetKind::Language;
        if (ext == ".scene" || ext == ".scenebin") return AssetKind::Scene;
        if (ext == ".prefab") return AssetKind::Prefab;

        return AssetKind::Unknown;
    }

    std::vector<AssetTask> collect_project_assets(const fs::path& projectRoot) {
        std::vector<AssetTask> tasks;
        fs::path assetsDir = projectRoot / "assets";

        for (fs::recursive_directory_iterator it(assetsDir), end; it != end; ++it) {
            if (it->is_directory())
                continue;

            fs::path relative = fs::relative(it->path(), projectRoot);
            AssetKind kind = classify_asset(relative);
            if (kind == AssetKind::Unknown)
                continue;

            tasks.push_back({relative.generic_string(), kind});
        }

        return tasks;
    }

    std::unordered_map<std::string, std::string> validate_assets(AssetManager& assets, Engine& sceneEngine, const std::vector<AssetTask>& tasks) {
        Loaders::register_default_loader<Texture>(assets);
        Loaders::register_default_loader<Font>(assets);
        Loaders::register_default_loader<Model>(assets);
        Loaders::register_default_loader<SoundBuffer>(assets);
        Loaders::register_default_loader<Animation>(assets);

        for (const AssetTask& task : tasks) {
            switch (task.kind) {
                case AssetKind::Texture: assets.queue<Texture>(task.key); break;
                case AssetKind::Font: assets.queue<Font>(task.key); break;
                case AssetKind::Model: assets.queue<Model>(task.key); break;
                case AssetKind::Sound: assets.queue<SoundBuffer>(task.key); break;
                case AssetKind::Animation: assets.queue<Animation>(task.key); break;
                case AssetKind::Language: break; // not validated, only packed
                case AssetKind::Scene: break; // validated separately below, load_scene() doesn't go through AssetManager's loader registry
                case AssetKind::Prefab: break; // not validated, plain JSON read on demand like a Scene but never through AssetManager
                case AssetKind::RML: break; // not validated, only packed
                case AssetKind::RCSS: break; // not validated, only packed
                default: break;
            }
        }

        assets.load();

        std::unordered_map<std::string, std::string> errors;
        for (const auto& err : assets.get_load_errors())
            errors[err.key] = describe(err.error);

        for (const AssetTask& task : tasks) {
            if (task.kind != AssetKind::Scene)
                continue;

            try {
                Scene scratch;
                FileHandle handle = HostFileSystem().open(task.key);

                if (fs::path(task.key).extension() == ".scenebin")
                    load_scene_binary(scratch, sceneEngine, assets, handle);
                else
                    load_scene(scratch, sceneEngine, assets, handle);
            } catch (const std::exception& e) {
                errors[task.key] = e.what();
            }
        }

        return errors;
    }
}
