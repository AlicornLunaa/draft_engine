#pragma once

#include "draft/build_tools/game_context.hpp"
#include "draft/ecs/scene.hpp"

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
