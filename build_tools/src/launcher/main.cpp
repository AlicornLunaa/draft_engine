#include "draft/asset/asset_manager.hpp"
#include "draft/build_tools/game_context.hpp"
#include "draft/core/application.hpp"
#include "draft/core/engine.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/interface/rmlui/rml_system.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"
#include "draft/util/logger.hpp"

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>

#if defined(DRAFT_STATIC_GAME_MODULE)
    // Release: the game's DRAFT_GAME_INFO/DRAFT_GAME_MODULE are compiled into this same
    // executable, so their exported functions are just ordinary symbols to call directly, no
    // module file, no dynamic loading.
    extern "C" void draft_game_info(Draft::GameInfo& info);
    extern "C" void draft_register_game(Draft::GameContext& context, Draft::Scene& scene);
#else
    #include "draft/build_tools/game_module_loader.hpp"
#endif

using namespace Draft;

namespace {
    // Reads the tiny JSON manifest a project's build points the launcher at: (Debug only) where
    // to find the compiled game module to load. Title/window size come from the module itself
    // (DRAFT_GAME_INFO), not from here.
    std::string load_module_path(const std::string& path){
        JSON json(HostFileSystem().open(path));
        return json.value("module", std::string());
    }
}

int main(int argc, char** argv){
    #if !defined(DRAFT_STATIC_GAME_MODULE)
        // Argument only needed if not Release build
        if(argc != 2){
            std::fprintf(stderr, "Usage: %s <manifest.json>\n", argv[0]);
            return 1;
        }
        
        std::unique_ptr<GameModuleLoader> module;

        try {
            std::string modulePathStr = load_module_path(argv[1]);
            // The module path in the manifest is relative to the manifest itself, resolved to an absolute path here.
            std::filesystem::path modulePath = std::filesystem::absolute(std::filesystem::path(argv[1]).parent_path() / modulePathStr);
            module = std::make_unique<GameModuleLoader>(modulePath);
        } catch(const std::exception& e){
            Logger::println(LogLevel::Critical, "Launcher", std::string("Failed to load game module: ") + e.what());
            return 1;
        }
    #endif

    #if defined(DRAFT_STATIC_GAME_MODULE)
        GameInfo info;
        draft_game_info(info);
    #else
        GameInfo info = module->game_info();
    #endif

    Engine engine;
    Application app(info.title, info.width, info.height);
    RmlUiSystem::set_clipboard_window(app.window);
    AssetManager assets;
    GameContext context{engine, app, assets};
    Scene scene;

    #if defined(DRAFT_STATIC_GAME_MODULE)
        draft_register_game(context, scene);
    #else
        module->register_game(context, scene);
    #endif

    app.set_scene(&scene);
    app.run();

    return 0;
}
