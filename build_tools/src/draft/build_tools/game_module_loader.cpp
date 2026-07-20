#include "draft/build_tools/game_module_loader.hpp"

#include <atomic>
#include <stdexcept>
#include <system_error>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace Draft {
    namespace {
        constexpr const char* GAME_INFO_SYMBOL = "draft_game_info";
        constexpr const char* REGISTER_SYMBOL = "draft_register_game";

        // A fresh, unique filename per load, so dlopen()/LoadLibrary() never sees the same path twice.
        // On POSIX, loading @p path directly would mean that once anything in it.
        std::filesystem::path make_load_copy_path(const std::filesystem::path& path){
            static std::atomic<uint64_t> counter{0};

            std::filesystem::path dir = path.parent_path() / ".reload";
            std::filesystem::create_directories(dir);

            return dir / (path.stem().string() + "_" + std::to_string(counter.fetch_add(1, std::memory_order_relaxed)) + path.extension().string());
        }
    }

    GameModuleLoader::GameModuleLoader(const std::filesystem::path& path){
        m_copyPath = make_load_copy_path(path);
        std::filesystem::copy_file(path, m_copyPath, std::filesystem::copy_options::overwrite_existing);

        #if defined(_WIN32)
            m_handle = LoadLibraryA(m_copyPath.string().c_str());
            if(!m_handle){
                std::error_code ec; std::filesystem::remove(m_copyPath, ec);
                throw std::runtime_error("GameModuleLoader: failed to load '" + path.string() + "'");
            }

            m_gameInfo = reinterpret_cast<GameInfoFn>(GetProcAddress(static_cast<HMODULE>(m_handle), GAME_INFO_SYMBOL));
            m_registerGame = reinterpret_cast<RegisterGameFn>(GetProcAddress(static_cast<HMODULE>(m_handle), REGISTER_SYMBOL));
        #else
            m_handle = dlopen(m_copyPath.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
            if(!m_handle){
                std::error_code ec; std::filesystem::remove(m_copyPath, ec);
                throw std::runtime_error("GameModuleLoader: failed to load '" + path.string() + "': " + dlerror());
            }

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

            std::error_code ec; std::filesystem::remove(m_copyPath, ec);
            throw std::runtime_error("GameModuleLoader: '" + path.string() + "' doesn't export draft_game_info()/draft_register_game() (missing DRAFT_GAME_INFO/DRAFT_GAME_MODULE?)");
        }
    }

    GameModuleLoader::GameModuleLoader(GameModuleLoader&& other) noexcept
        : m_handle(other.m_handle), m_gameInfo(other.m_gameInfo), m_registerGame(other.m_registerGame),
          m_copyPath(std::move(other.m_copyPath))
    {
        other.m_handle = nullptr;
        other.m_gameInfo = nullptr;
        other.m_registerGame = nullptr;
    }

    GameModuleLoader& GameModuleLoader::operator=(GameModuleLoader&& other) noexcept {
        if(this == &other)
            return *this;

        if(m_handle){
            #if defined(_WIN32)
                FreeLibrary(static_cast<HMODULE>(m_handle));
            #else
                dlclose(m_handle);
            #endif

            std::error_code ec; std::filesystem::remove(m_copyPath, ec);
        }

        m_handle = other.m_handle;
        m_gameInfo = other.m_gameInfo;
        m_registerGame = other.m_registerGame;
        m_copyPath = std::move(other.m_copyPath);
        other.m_handle = nullptr;
        other.m_gameInfo = nullptr;
        other.m_registerGame = nullptr;

        return *this;
    }

    GameModuleLoader::~GameModuleLoader(){
        if(m_handle){
            #if defined(_WIN32)
                FreeLibrary(static_cast<HMODULE>(m_handle));
            #else
                dlclose(m_handle);
            #endif

            std::error_code ec; std::filesystem::remove(m_copyPath, ec);
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
