#define GLFW_INCLUDE_NONE

#include "draft/asset/asset_manager.hpp"
#include "draft/asset/default_loaders.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/core/engine.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/ecs/scene_serializer.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Draft;
namespace fs = std::filesystem;

namespace {
    enum class AssetKind { Texture, Font, Model, Sound, Scene, Unknown };

    const char* kind_name(AssetKind kind){
        switch(kind){
            case AssetKind::Texture: return "Texture";
            case AssetKind::Font: return "Font";
            case AssetKind::Model: return "Model";
            case AssetKind::Sound: return "Sound";
            case AssetKind::Scene: return "Scene";
            default: return "Unknown";
        }
    }

    // True for any .json under an "assets/scenes/" directory, at any depth (e.g.
    // "assets/scenes/level1.json" or "assets/scenes/dungeon/level2.json").
    bool is_scene_json(const fs::path& projectRelativePath){
        std::vector<std::string> parts;
        for(const auto& part : projectRelativePath)
            parts.push_back(part.string());

        for(std::size_t i = 0; i + 1 < parts.size(); i++)
            if(parts[i] == "assets" && parts[i + 1] == "scenes")
                return true;

        return false;
    }

    AssetKind kind_for_path(const fs::path& projectRelativePath){
        std::string ext = projectRelativePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });

        if(ext == ".png" || ext == ".jpg" || ext == ".jpeg") return AssetKind::Texture;
        if(ext == ".ttf") return AssetKind::Font;
        if(ext == ".glb" || ext == ".gltf") return AssetKind::Model;
        if(ext == ".wav" || ext == ".ogg") return AssetKind::Sound;
        if(ext == ".json" && is_scene_json(projectRelativePath)) return AssetKind::Scene;

        return AssetKind::Unknown;
    }

    std::string describe(std::exception_ptr error){
        try {
            if(error)
                std::rethrow_exception(error);
        } catch(const std::exception& e){
            return e.what();
        } catch(...){}

        return "unknown error";
    }

    struct AssetTask {
        std::string key; // project-root-relative, forward slashes, e.g. "assets/textures/dev.png"
        AssetKind kind;
    };

    // Walks <projectRoot>/assets recursively, skipping anything whose extension isn't one of
    // the mapped types above (shaders, READMEs, etc. aren't validated by this phase).
    std::vector<AssetTask> collect_folder(const fs::path& projectRoot){
        std::vector<AssetTask> tasks;
        fs::path assetsDir = projectRoot / "assets";

        for(fs::recursive_directory_iterator it(assetsDir), end; it != end; ++it){
            if(it->is_directory())
                continue;

            fs::path relative = fs::relative(it->path(), projectRoot);
            AssetKind kind = kind_for_path(relative);
            if(kind == AssetKind::Unknown)
                continue;

            tasks.push_back({relative.generic_string(), kind});
        }

        return tasks;
    }
}

int main(int argc, char** argv){
    if(argc < 3 || std::string(argv[1]) != "validate"){
        std::fprintf(stderr, "Usage: %s validate <project-root> [asset-path]\n", argv[0]);
        return 1;
    }

    fs::path projectRoot = fs::absolute(argv[2]);
    if(!fs::is_directory(projectRoot)){
        Logger::println(LogLevel::Critical, "Validate", "Project root does not exist or is not a directory: " + projectRoot.string());
        return 1;
    }

    std::vector<AssetTask> tasks;

    if(argc >= 4){
        fs::path assetPath = fs::absolute(argv[3]);
        if(!fs::is_regular_file(assetPath)){
            Logger::println(LogLevel::Critical, "Validate", "Asset does not exist or is not a regular file: " + assetPath.string());
            return 1;
        }

        fs::path relative = fs::relative(assetPath, projectRoot);
        AssetKind kind = kind_for_path(relative);
        if(kind == AssetKind::Unknown){
            Logger::println(LogLevel::Critical, "Validate", "No validator registered for extension \"" + relative.extension().string() + "\"");
            return 1;
        }

        tasks.push_back({relative.generic_string(), kind});
    } else {
        if(!fs::is_directory(projectRoot / "assets")){
            Logger::println(LogLevel::Critical, "Validate", "No assets/ directory under " + projectRoot.string());
            return 1;
        }

        tasks = collect_folder(projectRoot);
    }

    // Texture/Font/Model construction all issue real GL calls (Font bakes glyphs, Model uploads
    // meshes/embedded textures), so a live GL context has to exist for the whole run
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    RenderWindow window(64, 64, "draft_buildtools_validate");

    // AssetManager resolves queue()/get() keys relative to the current working directory (via
    // DiskFileProvider), the same convention test_bench's "assets/..." keys already rely on.
    fs::current_path(projectRoot);

    AssetManager assets;
    Loaders::register_default_loader<Texture>(assets);
    Loaders::register_default_loader<Font>(assets);
    Loaders::register_default_loader<Model>(assets);
    Loaders::register_default_loader<SoundBuffer>(assets);

    for(const AssetTask& task : tasks){
        switch(task.kind){
            case AssetKind::Texture: assets.queue<Texture>(task.key); break;
            case AssetKind::Font: assets.queue<Font>(task.key); break;
            case AssetKind::Model: assets.queue<Model>(task.key); break;
            case AssetKind::Sound: assets.queue<SoundBuffer>(task.key); break;
            case AssetKind::Scene: break; // validated separately below, load_scene() doesn't go through AssetManager's loader registry
            default: break;
        }
    }

    assets.load();

    std::unordered_map<std::string, std::string> errors;
    for(const auto& err : assets.get_load_errors())
        errors[err.key] = describe(err.error);

    // load_scene() runs its own synchronous three-pass algorithm directly against a scratch
    // Scene/Engine, it isn't one of AssetManager's registered T loaders.
    Engine sceneEngine;
    for(const AssetTask& task : tasks){
        if(task.kind != AssetKind::Scene)
            continue;

        try {
            Scene scratch;
            load_scene(scratch, sceneEngine, assets, HostFileSystem().open(task.key));
        } catch(const std::exception& e){
            errors[task.key] = e.what();
        }
    }

    for(const AssetTask& task : tasks){
        auto it = errors.find(task.key);
        if(it == errors.end()){
            Logger::println(LogLevel::Info, kind_name(task.kind), task.key + " - OK");
        } else {
            Logger::println(LogLevel::Severe, kind_name(task.kind), task.key + " - FAILED: " + it->second);
        }
    }

    std::size_t failed = errors.size();
    Logger::println(failed == 0 ? LogLevel::Info : LogLevel::Critical, "Validate",
        std::to_string(tasks.size() - failed) + "/" + std::to_string(tasks.size()) + " assets valid");

    return failed == 0 ? 0 : 1;
}
