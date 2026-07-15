#include "draft/build_tools/game_module_loader.hpp"

#include <stdexcept>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace Draft {
    namespace {
        constexpr const char* GAME_INFO_SYMBOL = "draft_game_info";
        constexpr const char* REGISTER_SYMBOL = "draft_register_game";
    }

    GameModuleLoader::GameModuleLoader(const std::filesystem::path& path){
        #if defined(_WIN32)
            m_handle = LoadLibraryA(path.string().c_str());
            if(!m_handle)
                throw std::runtime_error("GameModuleLoader: failed to load '" + path.string() + "'");

            m_gameInfo = reinterpret_cast<GameInfoFn>(GetProcAddress(static_cast<HMODULE>(m_handle), GAME_INFO_SYMBOL));
            m_registerGame = reinterpret_cast<RegisterGameFn>(GetProcAddress(static_cast<HMODULE>(m_handle), REGISTER_SYMBOL));
        #else
            m_handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
            if(!m_handle)
                throw std::runtime_error("GameModuleLoader: failed to load '" + path.string() + "': " + dlerror());

            m_gameInfo = reinterpret_cast<GameInfoFn>(dlsym(m_handle, GAME_INFO_SYMBOL));
            m_registerGame = reinterpret_cast<RegisterGameFn>(dlsym(m_handle, REGISTER_SYMBOL));
        #endif

        if(!m_gameInfo || !m_registerGame){
            // Loaded fine but isn't a game module (missing DRAFT_GAME_INFO/DRAFT_GAME_MODULE), don't leak the handle.
            #if defined(_WIN32)
                FreeLibrary(static_cast<HMODULE>(m_handle));
            #else
                dlclose(m_handle);
            #endif

            throw std::runtime_error("GameModuleLoader: '" + path.string() + "' doesn't export draft_game_info()/draft_register_game() (missing DRAFT_GAME_INFO/DRAFT_GAME_MODULE?)");
        }
    }

    GameModuleLoader::~GameModuleLoader(){
        if(m_handle){
            #if defined(_WIN32)
                FreeLibrary(static_cast<HMODULE>(m_handle));
            #else
                dlclose(m_handle);
            #endif
        }
    }

    GameInfo GameModuleLoader::game_info() const {
        GameInfo info;
        m_gameInfo(info);
        return info;
    }

    void GameModuleLoader::register_game(GameContext& context, Scene& scene) const {
        m_registerGame(context, scene);
    }
}
