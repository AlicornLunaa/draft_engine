#pragma once

#include "draft/build_tools/game_context.hpp"
#include "draft/ecs/scene.hpp"

#include <filesystem>

namespace Draft {
    /**
     * @brief Loads a compiled game module and calls its registration entry point. The module
     * must be a shared library built with draft_add_game_module() (CMake) that exports
     * draft_register_game() via DRAFT_GAME_MODULE (see game_module.hpp).
     *
     * Debug-only path for now. Release links a game statically instead, with no dynamic loading
     * involved.
     */
    class GameModuleLoader {
    public:
        /**
         * @throws std::runtime_error if @p path can't be loaded or doesn't export
         * draft_register_game()/draft_game_info().
         */
        explicit GameModuleLoader(const std::filesystem::path& path);

        GameModuleLoader(const GameModuleLoader&) = delete;
        GameModuleLoader& operator=(const GameModuleLoader&) = delete;

        ~GameModuleLoader();

        /**
         * @brief Calls the module's DRAFT_GAME_INFO function.
         */
        GameInfo game_info() const;

        /**
         * @brief Calls the module's registration function with @p context and @p scene.
         */
        void register_game(GameContext& context, Scene& scene) const;

    private:
        using GameInfoFn = void(*)(GameInfo&);
        using RegisterGameFn = void(*)(GameContext&, Scene&);

        void* m_handle = nullptr;
        GameInfoFn m_gameInfo = nullptr;
        RegisterGameFn m_registerGame = nullptr;
    };
}
