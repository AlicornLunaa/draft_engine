#include "draft/editor/game_module_watcher.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/host_file_system.hpp"

namespace Draft {
    GameModuleWatcher::GameModuleWatcher(std::filesystem::path path) : m_path(std::move(path)) {
        FileHandle handle = HostFileSystem().open(m_path);
        if(handle.exists())
            m_lastModified = handle.last_modified();
    }

    bool GameModuleWatcher::poll(){
        FileHandle handle = HostFileSystem().open(m_path);
        if(!handle.exists())
            return false;

        Time modified = handle.last_modified();
        if(m_lastModified.has_value() && modified == *m_lastModified)
            return false;

        m_lastModified = modified;
        return true;
    }
}
