#pragma once

#include "draft/build_tools/game_context.hpp" // IWYU pragma: keep
#include "draft/ecs/scene.hpp" // IWYU pragma: keep

#if defined(_WIN32)
    #define DRAFT_GAME_MODULE_EXPORT __declspec(dllexport)
#else
    #define DRAFT_GAME_MODULE_EXPORT
#endif

/**
 * @def DRAFT_GAME_MODULE(fn)
 * @brief Exports @p fn as this shared library's game module entry point, so GameModuleLoader
 * can find and call it after loading this library. @p fn must be callable as
 * fn(Draft::GameContext&, Draft::Scene&): register the game's component/system types on
 * context.engine, then populate scene with whatever the game starts with.
 *
 * @code
 * void register_game(Draft::GameContext& context, Draft::Scene& scene){
 *     context.engine.systems().register_system<PlayerMovement>([](Draft::Scene&){
 *         return std::make_unique<PlayerMovement>();
 *     });
 *     scene.get_systems().add<PlayerMovement>();
 * }
 * DRAFT_GAME_MODULE(register_game)
 * @endcode
 */
#define DRAFT_GAME_MODULE(fn) \
    extern "C" DRAFT_GAME_MODULE_EXPORT void draft_register_game(Draft::GameContext& context, Draft::Scene& scene){ \
        fn(context, scene); \
    }

/**
 * @def DRAFT_GAME_INFO(title, width, height)
 * @brief Exports the game's title and default window size, so the launcher can read them before
 * Application even exists (it needs a title/size to construct). Goes alongside DRAFT_GAME_MODULE
 * in the same game module.
 *
 * @code
 * DRAFT_GAME_INFO("My Game", 1280, 720)
 * DRAFT_GAME_MODULE(register_game)
 * @endcode
 */
#define DRAFT_GAME_INFO(gameTitle, gameWidth, gameHeight) \
    extern "C" DRAFT_GAME_MODULE_EXPORT void draft_game_info(Draft::GameInfo& info){ \
        info = Draft::GameInfo{gameTitle, gameWidth, gameHeight}; \
    }
