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

        // First sight of this mtime (or it moved again since the last time we noticed it):
        // reset the settle timer rather than reporting the change yet.
        if(!m_pendingModified.has_value() || modified != *m_pendingModified){
            m_pendingModified = modified;
            m_pendingClock.restart();
            return false;
        }

        return m_pendingClock.get_elapsed_time() >= Time::milliseconds(1000);
    }
}
