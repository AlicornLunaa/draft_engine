#include "draft/asset/asset_manager.hpp"
#include "draft/build_tools/game_context.hpp"
#include "draft/core/application.hpp"
#include "draft/core/engine.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"
#include "draft/util/logger.hpp"

#include <cstdio>
#include <filesystem>
#include <string>

#if defined(DRAFT_STATIC_GAME_MODULE)
    // Release: the game's DRAFT_GAME_MODULE is compiled into this same executable, so its exported
    // function is just an ordinary symbol to call directly, no module file, no dynamic loading.
    extern "C" void draft_register_game(Draft::GameContext& context, Draft::Scene& scene);
#else
    #include "draft/build_tools/game_module_loader.hpp"
#endif

using namespace Draft;

namespace {
    struct LaunchManifest {
        std::string title = "Draft Game";
        unsigned int width = 1280;
        unsigned int height = 720;
        std::string module;
    };

    // Reads the small JSON manifest a project's build points the launcher at: window title and
    // size, and (Debug only) where to find the compiled game module to load.
    LaunchManifest load_manifest(const std::string& path){
        JSON json(HostFileSystem().open(path));

        LaunchManifest manifest;
        manifest.title = json.value("title", manifest.title);
        manifest.width = json.value("width", manifest.width);
        manifest.height = json.value("height", manifest.height);
        manifest.module = json.value("module", manifest.module);
        return manifest;
    }
}

int main(int argc, char** argv){
    if(argc != 2){
        std::fprintf(stderr, "Usage: %s <manifest.json>\n", argv[0]);
        return 1;
    }

    LaunchManifest manifest;

    try {
        manifest = load_manifest(argv[1]);
    } catch(const std::exception& e){
        Logger::println(LogLevel::Critical, "Launcher", std::string("Failed to read manifest: ") + e.what());
        return 1;
    }

    Engine engine;
    Application app(manifest.title, manifest.width, manifest.height);
    AssetManager assets;
    GameContext context{engine, app, assets};
    Scene scene;

    try {
        #if defined(DRAFT_STATIC_GAME_MODULE)
            draft_register_game(context, scene);
        #else
            // The module path in the manifest is relative to the manifest itself, resolved to an
            // absolute path here.
            std::filesystem::path modulePath = std::filesystem::absolute(std::filesystem::path(argv[1]).parent_path() / manifest.module);
            GameModuleLoader module(modulePath);
            module.register_game(context, scene);
        #endif
    } catch(const std::exception& e){
        Logger::println(LogLevel::Critical, "Launcher", std::string("Failed to load game module: ") + e.what());
        return 1;
    }

    app.set_scene(&scene);
    app.run();

    return 0;
}
